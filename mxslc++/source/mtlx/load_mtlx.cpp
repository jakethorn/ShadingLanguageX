//
// Created by jaket on 30/12/2025.
//

#include "load_mtlx.h"
#include <MaterialXFormat/Util.h>

#include "CompileError.h"
#include "runtime/Scope.h"
#include "runtime/TypeInfo.h"
#include "expressions/NullExpression.h"
#include "runtime/Function.h"
#include "utils/io_utils.h"
#include "utils/str_utils.h"

namespace
{
    const unordered_set<string> default_node_defs = {"ND_randomfloat_float"s};

    Parameter to_parameter(const Runtime& runtime, const mx::InputPtr& i, const size_t index)
    {
        const TypeInfoPtr type = runtime.scope().get_type(i->getType());
        const string& name = i->getName();
        ExprPtr expr = std::make_unique<NullExpression>(runtime);
        return Parameter{runtime, ModifierList{}, type, name, std::move(expr), index};
    }

    ParameterList get_parameters(const Runtime& runtime, const mx::NodeDefPtr& nd)
    {
        vector<Parameter> params;
        params.reserve(nd->getInputCount());
        for (const mx::InputPtr& i : nd->getInputs())
            params.push_back(to_parameter(runtime, i, params.size()));

        return ParameterList{std::move(params)};
    }

    TypeInfoPtr get_type(const Runtime& runtime, const mx::NodeDefPtr& nd)
    {
        vector<TypeInfoPtr> subtypes;
        subtypes.reserve(nd->getOutputCount());
        for (const mx::OutputPtr& o : nd->getActiveOutputs())
            subtypes.push_back(std::make_shared<TypeInfo>(o->getType()));

        const TypeInfoPtr type = subtypes.size() == 1 ? subtypes.at(0) : std::make_shared<TypeInfo>(std::move(subtypes));
        return runtime.scope().resolve_type(type);
    }

    vector<string> get_output_names(const mx::NodeDefPtr& nd)
    {
        vector<string> names;
        names.reserve(nd->getOutputCount());
        for (const mx::OutputPtr& o : nd->getActiveOutputs())
            names.push_back(o->getName());
        return names;
    }

    FuncPtr to_function(const Runtime& runtime, const mx::NodeDefPtr& nd)
    {
        const Scope& scope = runtime.scope();

        ModifierList mods = {};
        if (contains(default_node_defs, nd->getName()))
            mods.add("default"s);

        TypeInfoPtr type = get_type(runtime, nd);
        const string& name = nd->getNodeString();
        const string template_type_name = get_postfix(nd->getName(), '_');
        TypeInfoPtr template_type = scope.has_type(template_type_name) ? scope.get_type(template_type_name) : nullptr;
        ParameterList params = get_parameters(runtime, nd);
        vector<string> output_names = get_output_names(nd);
        FuncPtr func = std::make_shared<Function>(std::move(mods), std::move(type), name, std::move(template_type), std::move(params), std::move(output_names));
        func->init(runtime);
        return func;
    }
}

void load_library(const Runtime& runtime, const mx::DocumentPtr& doc)
{
    Scope& scope = runtime.scope();

    for (const mx::TypeDefPtr& td : doc->getTypeDefs())
    {
        scope.add_basic_type(td->getName());
    }

    for (const mx::NodeDefPtr& nd : doc->getNodeDefs())
    {
        if (nd->hasVersionString() and not nd->getDefaultVersion())
            continue;
        scope.add_function(to_function(runtime, nd));
    }
}

void load_library(const Runtime& runtime, const fs::path& filepath)
{
    const mx::DocumentPtr doc = mx::createDocument();
    mx::readFromXmlFile(doc, filepath.string());
    load_library(runtime, doc);
}

void load_materialx_library(const Runtime& runtime, const string& version)
{
    const mx::FilePathVec fpv{version};
    const fs::path lib_dir = get_libraries_dir();
    const mx::FileSearchPath fsp{lib_dir.string()};
    const mx::DocumentPtr doc = mx::createDocument();
    const mx::StringSet loaded = mx::loadLibraries(fpv, fsp, doc);
    if (loaded.empty())
        throw CompileError{"MaterialX version '" + version + "' is not supported"};
    load_library(runtime, doc);
}

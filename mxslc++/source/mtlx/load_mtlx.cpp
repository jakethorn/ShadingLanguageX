//
// Created by jaket on 30/12/2025.
//

#include "load_mtlx.h"
#include <MaterialXFormat/Util.h>

#include "CompileError.h"
#include "runtime/Scope.h"
#include "expressions/NullExpression.h"
#include "runtime/Function.h"
#include "utils/io_utils.h"
#include "utils/str_utils.h"

namespace
{
    Parameter to_parameter(const Runtime& runtime, const mx::InputPtr& i, const size_t index)
    {
        const string& type = i->getType();
        const string& name = i->getName();
        ExprPtr expr = std::make_unique<NullExpression>(runtime);
        return Parameter{{}, type, name, std::move(expr), index};
    }

    ParameterList get_parameters(const Runtime& runtime, const mx::NodeDefPtr& nd)
    {
        vector<Parameter> params;
        params.reserve(nd->getInputCount());
        for (const mx::InputPtr& i : nd->getInputs())
            params.push_back(to_parameter(runtime, i, params.size()));

        return ParameterList{std::move(params)};
    }

    Type get_type_name(const mx::NodeDefPtr& nd)
    {
        vector<Type> subtypes;
        subtypes.reserve(nd->getOutputCount());
        for (const mx::OutputPtr& o : nd->getActiveOutputs())
            subtypes.emplace_back(o->getType());
        return Type{std::move(subtypes)};
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

        Type type = get_type_name(nd);
        const string name = nd->getNodeString();
        optional<string> template_type = get_postfix(name, '_');
        if (not scope.has_type(template_type.value()))
            template_type = std::nullopt;
        ParameterList params = get_parameters(runtime, nd);
        vector<string> output_names = get_output_names(nd);
        return std::make_shared<Function>(std::move(type), name, std::move(template_type), std::move(params), std::move(output_names));
    }
}

void load_library(const Runtime& runtime, const mx::DocumentPtr& doc)
{
    Scope& scope = runtime.scope();

    for (const mx::TypeDefPtr& td : doc->getTypeDefs())
    {
        scope.add_type(td->getName());
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

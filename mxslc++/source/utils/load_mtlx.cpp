//
// Created by jaket on 30/12/2025.
//

#include "../../include/utils/load_mtlx.h"
#include <MaterialXFormat/Util.h>

#include "statements/Statement.h"
#include "runtime/Scope.h"
#include "runtime/Type.h"
#include "expressions/NullExpression.h"
#include "runtime/Function.h"
#include "runtime/Runtime.h"
#include "utils/io_utils.h"
#include "utils/string_utils.h"
#include "errors/CompileError.h"
#include "expressions/interface.h"
#include "runtime/interface.h"

namespace mxslc
{
    using container_utils::contains;

    namespace
    {
        const unordered_set<string> DEFAULT_NODE_DEFS = {"ND_randomfloat_float", "ND_randomcolor_float"};

        Parameter to_parameter(const mx::InputPtr& i, const size_t index)
        {
            const TypePtr type = Runtime::get().scope().get_type(i->getType());
            const string& name = i->getName();
            ExprPtr expr = create_expression<NullExpression>();
            return Parameter{AttributeList{}, ModifierList{}, type, name, std::move(expr), index};
        }

        ParameterList get_parameters(const mx::NodeDefPtr& nd)
        {
            vector<Parameter> params;
            params.reserve(nd->getInputCount());
            for (const mx::InputPtr& i : nd->getActiveInputs())
                params.push_back(to_parameter(i, params.size()));

            return ParameterList{std::move(params)};
        }

        TypePtr get_type(const mx::NodeDefPtr& nd)
        {
            vector<TypePtr> subtypes;
            subtypes.reserve(nd->getOutputCount());
            for (const mx::OutputPtr& o : nd->getActiveOutputs())
                subtypes.push_back(create_type(o->getType()));

            const TypePtr type = subtypes.size() == 1 ? subtypes.at(0) : std::make_shared<Type>(std::move(subtypes));
            return Runtime::get().scope().resolve_type(type);
        }

        vector<string> get_output_names(const mx::NodeDefPtr& nd)
        {
            vector<string> names;
            names.reserve(nd->getOutputCount());
            for (const mx::OutputPtr& o : nd->getActiveOutputs())
                names.push_back(o->getName());
            return names;
        }

        FuncPtr to_function(const mx::NodeDefPtr& nd)
        {
            const Scope& scope = Runtime::get().scope();

            ModifierList mods;
            if (contains(DEFAULT_NODE_DEFS, nd->getName()))
                mods.add(TokenType::Default);

            TypePtr type = get_type(nd);
            const string& name = nd->getNodeString();
            const string template_type_name = string_utils::get_postfix(nd->getName(), '_');
            TypePtr template_type = scope.has_type(template_type_name) ? scope.get_type(template_type_name) : nullptr;
            ParameterList params = get_parameters(nd);
            vector<string> output_names = get_output_names(nd);
            FuncPtr func = create_function(std::move(mods), std::move(type), name, std::move(template_type), std::move(params), nd);
            func->init();
            return func;
        }
    }

    void load_library(const mx::DocumentPtr& doc)
    {
        Scope& scope = Runtime::get().scope();

        for (const mx::TypeDefPtr& td : doc->getTypeDefs())
        {
            scope.add_primitive_type(td->getName());
        }

        for (const mx::NodeDefPtr& nd : doc->getNodeDefs())
        {
            if (nd->hasVersionString() and not nd->getDefaultVersion())
                continue;
            scope.add_function(to_function(nd));
        }
    }

    void load_library(const fs::path& filepath)
    {
        const mx::DocumentPtr doc = mx::createDocument();
        mx::readFromXmlFile(doc, filepath.string());
        load_library(doc);
    }

    mx::DocumentPtr get_materialx_library(const string& version, const vector<fs::path>& include_dirs)
    {
        string searched_dirs;

        for (const fs::path& include_dir : include_dirs)
        {
            const fs::path lib_dir = include_dir / "libraries";
            searched_dirs += lib_dir.string() + "\n";

            if (not fs::is_directory(lib_dir))
                continue;

            const mx::FilePathVec fpv{version};
            const mx::FileSearchPath fsp{lib_dir.string()};
            const mx::DocumentPtr doc = mx::createDocument();
            const mx::StringSet loaded = mx::loadLibraries(fpv, fsp, doc);
            if (not loaded.empty())
                return doc;
        }

        throw CompileError{"MaterialX version " + version + " libraries could not be found.\nSearched directories:\n" + searched_dirs};
    }
}

//
// Created by jaket on 30/12/2025.
//

#include "utils/load_mtlx.h"

#include <MaterialXFormat/Util.h>

#include "expressions/interface.h"
#include "expressions/NullExpression.h"
#include "runtime/interface.h"
#include "runtime/ModifierList.h"
#include "runtime/ParameterList.h"
#include "runtime/Runtime.h"
#include "runtime/Scope.h"
#include "runtime/Function.h"
#include "runtime/Type.h"
#include "utils/container_utils.h"
#include "utils/string_utils.h"

namespace mxslc
{
    using container_utils::contains;

    namespace
    {
        string get_return_type_key(const mx::NodeDefPtr& nd)
        {
            const vector<mx::OutputPtr> outputs = nd->getActiveOutputs();

            if (outputs.empty())
                return nd->getType();

            if (outputs.size() == 1)
                return outputs.front()->getType();

            string key;
            for (const mx::OutputPtr& output : outputs)
            {
                if (not key.empty())
                    key += '|';
                key += output->getType();
            }

            return key;
        }

        // Scan every nodedef in the loaded MaterialX document and pick the
        // first definition read for each node category and return type pair as
        // the "default", but only when there are multiple definitions for that
        // pair. This keeps defaults focused on ambiguous overload groups.
        // Versioned nodedefs are reduced to their default version, matching how
        // they are loaded.
        unordered_set<string> get_default_node_defs(const mx::DocumentPtr& doc)
        {
            unordered_map<string, unordered_map<string, string>> best_by_category_and_type;
            unordered_map<string, unordered_map<string, size_t>> counts_by_category_and_type;
            unordered_set<string> defaults;

            for (const mx::NodeDefPtr& nd : doc->getNodeDefs())
            {
                if (nd->hasVersionString() and not nd->getDefaultVersion())
                    continue;

                const string& category = nd->getNodeString();
                const string return_type = get_return_type_key(nd);

                auto& best_by_type = best_by_category_and_type[category];
                auto& counts_by_type = counts_by_category_and_type[category];

                if (not contains(best_by_type, return_type))
                {
                    best_by_type.emplace(return_type, nd->getName());
                }
                else
                {
                    string& best_name = best_by_type.at(return_type);
                    if (nd->getName() < best_name)
                        best_name = nd->getName();
                }

                ++counts_by_type[return_type];
            }

            for (const auto& [category, counts_by_type] : counts_by_category_and_type)
            {
                const auto& best_by_type = best_by_category_and_type.at(category);
                for (const auto& [return_type, count] : counts_by_type)
                {
                    if (count > 1)
                        defaults.insert(best_by_type.at(return_type));
                }
            }

            return defaults;
        }

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

            const TypePtr type = subtypes.size() == 1 ? subtypes.at(0) : create_type(std::move(subtypes));
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

        FuncPtr to_function(const mx::NodeDefPtr& nd, const unordered_set<string>& default_node_defs)
        {
            const Scope& scope = Runtime::get().scope();

            ModifierList mods;
            if (contains(default_node_defs, nd->getName()))
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

        const unordered_set<string> default_node_defs = get_default_node_defs(doc);

        for (const mx::NodeDefPtr& nd : doc->getNodeDefs())
        {
            if (nd->hasVersionString() and not nd->getDefaultVersion())
                continue;
            scope.add_function(to_function(nd, default_node_defs));
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

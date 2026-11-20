//
// Created by jaket on 04/11/2026.
//

#include "reflection/Reflection.h"

#include <algorithm>

#include "Primitive.h"
#include "TokenType.h"
#include "expressions/Expression.h"
#include "runtime/Function.h"
#include "runtime/Parameter.h"
#include "runtime/Scope.h"
#include "runtime/Type.h"
#include "runtime/utils/TypeName.h"
#include "utils/string_utils.h"

namespace mxslc::reflection
{
    using runtime_utils::TypeName;
    using string_utils::starts_with;

    namespace
    {
        template<typename Info>
        void sort_and_remove_duplicates(vector<Info>& infos)
        {
            std::stable_sort(infos.begin(), infos.end(), [](const Info& a, const Info& b) { return a.name < b.name; });
            const auto it = std::unique(infos.begin(), infos.end(), [](const Info& a, const Info& b) { return a.name == b.name; });
            infos.erase(it, infos.end());
        }

        Source get_source(const Scope& scope)
        {
            if (scope.name() == "mtlx_stdlib")
                return Source::MaterialX;
            if (scope.name() == "mxsl_stdlib")
                return Source::SlxStandardLibrary;
            return Source::UserDefined;
        }

        bool is_callable_function_name(const string& name)
        {
            return not TokenType{name}.is_keyword() and not starts_with(name, "__");
        }

        vector<FieldInfo> get_field_infos(const TypePtr& type)
        {
            vector<FieldInfo> fields;
            for (const Field& field : type->fields())
                fields.push_back(FieldInfo{field.name(), field.type()->to_string()});
            return fields;
        }

        bool is_materialx_node(const Function& func)
        {
            return not func.is_defined() and func.node_def() != nullptr;
        }

        vector<string> get_modifier_names(const ModifierList& mods)
        {
            vector<string> names;
            for (const TokenType& mod : mods)
                names.push_back(mod.to_string());
            std::sort(names.begin(), names.end());
            return names;
        }

        string get_default_expression(const Parameter& param)
        {
            if (not param.has_default_value())
                return "";
            string expression = param.default_value()->to_string();
            return expression == "null" ? "" : expression;
        }

        string get_default_expression(const mx::InputPtr& input)
        {
            if (input->hasDefaultGeomPropString())
                return "geomprop(" + input->getDefaultGeomPropString() + ")";
            const mx::ValuePtr value = input->getValue();
            if (not value)
                return "";
            const Primitive primitive{value};
            return primitive.is_null() ? input->getValueString() : primitive.to_string();
        }

        ParameterInfo get_parameter_info(const Function& func, const Parameter& param)
        {
            ParameterInfo info;
            info.name = param.name();
            info.type = param.type()->to_string();
            info.default_expression = get_default_expression(param);
            info.modifiers = get_modifier_names(param.modifiers());

            if (is_materialx_node(func))
            {
                if (const mx::InputPtr input = func.node_def()->getActiveInput(param.name()))
                {
                    info.default_expression = get_default_expression(input);
                    if (input->getIsUniform())
                        info.modifiers = {"uniform"};
                }
            }

            return info;
        }

        OverloadInfo get_overload_info(const Function& func)
        {
            OverloadInfo info;
            for (const Parameter& param : func.parameters())
                info.parameters.push_back(get_parameter_info(func, param));
            info.return_type = func.return_type()->to_string();
            if (func.has_template_type())
                info.template_type = func.template_type()->to_string();
            info.modifiers = get_modifier_names(func.modifiers());
            if (func.defining_scope())
                info.source = get_source(*func.defining_scope());
            if (func.has_class_type())
                info.class_type = func.class_type()->name();
            info.path = func.token().path();
            info.line = func.token().line();
            return info;
        }

        string get_signature(const Function& func)
        {
            string signature = func.name() + "(";
            for (const Parameter& param : func.parameters())
                signature += param.type()->to_string() + ",";
            return signature + ")" + func.return_type()->to_string();
        }

        vector<FunctionInfo> get_function_infos(const vector<FuncPtr>& funcs)
        {
            vector<FunctionInfo> infos;
            unordered_map<string, size_t> name_indices;
            unordered_map<string, const Scope*> signature_scopes;

            for (const FuncPtr& func : funcs)
            {
                if (not is_callable_function_name(func->name()))
                    continue;

                const auto [signature, is_new_signature] = signature_scopes.try_emplace(get_signature(*func), func->defining_scope());
                if (not is_new_signature and signature->second != func->defining_scope())
                    continue;

                const auto [name_index, is_new_name] = name_indices.try_emplace(func->name(), infos.size());
                if (is_new_name)
                    infos.push_back(FunctionInfo{func->name(), {}});
                infos[name_index->second].overloads.push_back(get_overload_info(*func));
            }

            sort_and_remove_duplicates(infos);
            return infos;
        }
    }

    void Reflection::add_global(string name, TypePtr type, VarPtr value)
    {
        // globals in different scopes are the same global
        for (const GlobalInfo& global : globals_)
            if (global.name == name)
                return;
        globals_.push_back(GlobalInfo{std::move(name), std::move(type), std::move(value)});
    }

    void Reflection::collect_info()
    {
        types_.clear();
        for (const auto& [name, type] : scope().get_all_types())
        {
            TypeInfo info;
            info.name = name;
            info.is_alias = name != type->name();
            info.source = get_source(scope().get_type_defining_scope(name));
            if (info.is_alias)
                info.aliased_type = type->to_string();
            info.fields = get_field_infos(type);
            info.methods = get_function_infos(type->methods());
            info.path = type->token().path();
            info.line = type->token().line();
            types_.push_back(std::move(info));
        }
        for (const string& name : TypeName::Reserved)
        {
            TypeInfo info;
            info.name = name;
            info.source = Source::SlxStandardLibrary;
            types_.push_back(std::move(info));
        }
        sort_and_remove_duplicates(types_);

        functions_ = get_function_infos(scope().get_all_functions());
    }
}

//
// Created by jaket on 16/04/2026.
//

#include "statements/FunctionDefinition.h"

#include "runtime/Function.h"
#include "runtime/Runtime.h"
#include "runtime/Scope.h"
#include "runtime/Type.h"
#include "expressions/Expression.h"
#include "errors/CompileError.h"
#include "runtime/interface.h"
#include "runtime/utils/monomorphize.h"
#include "statements/interface.h"

namespace mxslc::statements
{
    using container_utils::contains;

    FunctionDefinition::FunctionDefinition(
        ModifierList mods,
        TypePtr type,
        string name,
        vector<TypePtr> template_types,
        optional<ParameterList> params,
        StmtPtr body,
        Token token
    ) : Statement{std::move(token)},
        mods_{std::move(mods)},
        type_{std::move(type)},
        name_{std::move(name)},
        template_types_{std::move(template_types)},
        params_{std::move(params)},
        body_{std::move(body)}
    {

    }

    void FunctionDefinition::set_attributes(AttributeList attrs)
    {
        attrs_ = std::move(attrs);
    }

    StmtPtr FunctionDefinition::monomorphize(const TypePtr& template_type) const
    {
        if (is_templated())
            throw CompileError{"Nested templated functions is not supported"};

        return create_statement<FunctionDefinition>(
            mods_,
            runtime_utils::monomorphize(type_, template_type),
            name_,
            template_types_,
            runtime_utils::monomorphize(params_, template_type),
            runtime_utils::monomorphize(body_, template_type),
            token_
        );
    }

    void FunctionDefinition::create_functions()
    {
        if (not funcs_.empty())
            return;

        if (is_templated())
        {
            validate_template_types();

            for (const TypePtr& template_type : template_types_)
            {
                funcs_.push_back(create_function(
                    mods_,
                    runtime_utils::monomorphize(type_, template_type),
                    name_,
                    template_type,
                    runtime_utils::monomorphize(params_, template_type),
                    runtime_utils::monomorphize(body_, template_type)
                ));
            }
        }
        else
        {
            funcs_.push_back(create_function(
                std::move(mods_),
                std::move(type_),
                std::move(name_),
                nullptr,
                std::move(params_),
                std::move(body_)
            ));
        }
    }

    void FunctionDefinition::init()
    {
        create_functions();

        for (const FuncPtr& func : funcs_)
        {
            func->init();
            scope().add_function(func);

            if (not func->is_inline())
                serializer().write_node_def_graph(func, attrs_);
        }
    }

    void FunctionDefinition::execute_impl() const
    {
        for (const FuncPtr& func : funcs_)
        {
            if (not scope().has_function(func))
            {
                scope().add_function(func);

                if (not func->is_inline())
                    func->update_nonlocal_variables();
            }
        }
    }

    void FunctionDefinition::validate_template_types()
    {
        if (not is_templated())
            return;

        vector<TypePtr> validated_types;
        for (TypePtr& template_type : template_types_)
        {
            template_type = scope().resolve_type(template_type);
            if (not contains(validated_types, template_type))
                validated_types.push_back(template_type);
        }

        template_types_ = std::move(validated_types);
    }

    string FunctionDefinition::to_string() const
    {
        if (not is_templated())
            return funcs_.front()->to_string();

        string mods_string = mods_.to_string();
        if (not mods_string.empty())
            mods_string += ' ';

        string result;
        result += mods_string;
        result += type_->to_string();
        result += ' ' + name_;
        if (not template_types_.empty())
            result += '<' + join(template_types_, ", ") + '>';
        if (params_)
            result += '(' + params_->to_string() + ')';
        result += '\n';
        result += body_->to_string();

        return result;
    }
}

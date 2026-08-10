//
// Created by jaket on 16/04/2026.
//

#include "runtime/Function.h"

#include <cassert>

#include "expressions/Expression.h"
#include "statements/Statement.h"
#include "runtime/Scope.h"
#include "runtime/Type.h"
#include "runtime/variables/Variable.h"
#include "runtime/utils/type_cast.h"
#include "errors/CompileError.h"
#include "serialize/Serializer.h"

namespace mxslc::runtime
{
    Function::Function(
        ModifierList mods,
        TypePtr return_type,
        string name,
        TypePtr template_type,
        ParameterList params,
        mx::NodeDefPtr node_def
    ) : mods_{std::move(mods)},
        return_type_{std::move(return_type)},
        name_{std::move(name)},
        template_type_{std::move(template_type)},
        params_{std::move(params)}
    {
        set_node_def(std::move(node_def));
    }

    Function::Function(
        ModifierList mods,
        TypePtr return_type,
        string name,
        TypePtr template_type,
        optional<ParameterList> params,
        StmtPtr body,
        ExprPtr return_expr
    ) : mods_{std::move(mods)},
        return_type_{std::move(return_type)},
        name_{std::move(name)},
        template_type_{std::move(template_type)},
        params_{std::move(params).value_or(ParameterList{})},
        body_{std::move(body)},
        return_expr_{std::move(return_expr)},
        is_parameterless_{not params.has_value()}
    {
        mods_.validate(TokenType::Inline, TokenType::Default, TokenType::Comptime);

        if (return_type_->is_void() and return_expr_ != nullptr)
            throw CompileError{"Void function '" + name_ + "' has a return statement"};
        if (not return_type_->is_void() and return_expr_ == nullptr)
            throw CompileError{"Non-void function '" + name_ + "' does not have a return statement"};

        if (return_type_->is_void() and is_parameterless_)
            throw CompileError{"Parameterless function '" + name_ + "' cannot be void"};
    }

    Function::Function(Function&& other) noexcept
        : mods_{std::move(other.mods_)},
        return_type_{std::move(other.return_type_)},
        name_{std::move(other.name_)},
        template_type_{std::move(other.template_type_)},
        params_{std::move(other.params_)},
        body_{std::move(other.body_)},
        return_expr_{std::move(other.return_expr_)},
        node_def_{std::move(other.node_def_)},
        is_initialized_{other.is_initialized_},
        nonlocal_inputs_{std::move(other.nonlocal_inputs_)},
        nonlocal_outputs_{std::move(other.nonlocal_outputs_)}
    {

    }

    Function::~Function() = default;

    bool Function::is_void() const
    {
        return return_type_->is_void();
    }

    size_t Function::min_arity() const
    {
        size_t arity = 0;
        for (const Parameter& param : params_)
        {
            if (not param.has_default_value())
                ++arity;
        }

        return arity;
    }

    void Function::set_node_def(mx::NodeDefPtr node_def)
    {
        assert(node_def_ == nullptr);
        assert(not is_parameterless_);
        node_def_ = std::move(node_def);
    }

    void Function::set_node_graph(mx::NodeGraphPtr node_graph)
    {
        assert(node_graph_ == nullptr);
        node_graph_ = std::move(node_graph);
    }

    vector<string> Function::output_names() const
    {
        if (is_defined())
            return {};

        vector<string> names;
        names.reserve(node_def_->getOutputCount());
        for (const mx::OutputPtr& o : node_def_->getActiveOutputs())
            names.push_back(o->getName());
        return names;
    }

    void Function::init()
    {
        return_type_ = scope().resolve_type(return_type_);

        if (template_type_)
            template_type_ = scope().resolve_type(template_type_);

        params_.init();

        is_initialized_ = true;
    }

    VarPtr Function::invoke() const
    {
        if (parameterless_cache_)
            return parameterless_cache_;

        serializer().begin_comptime(is_comptime());

        body_->execute();

        VarPtr return_value;
        if (is_void())
        {
            return_value = nullptr;
        }
        else
        {
            return_expr_->init(return_type_);
            return_value = type_cast(return_type_, return_expr_->evaluate(), /*force*/true);
            if (is_parameterless_)
                parameterless_cache_ = return_value;
        }

        serializer().end_comptime();

        return return_value;
    }

    void Function::update_nonlocal_variables()
    {
        for (VarPtr& var : nonlocal_inputs_)
            var = scope().get_variable(var->name());
        for (VarPtr& var : nonlocal_outputs_)
            var = scope().get_variable(var->name());
    }

    string Function::header() const
    {
        string mods_string = mods_.to_string();
        if (not mods_string.empty())
            mods_string += ' ';

        string result;
        result += mods_string;
        result += return_type_->to_string() + ' ';
        if (has_class_type())
            result += class_type()->to_string() + '.';
        result += name_;
        if (has_template_type())
            result += '<' + template_type_->to_string() + '>';
        if (not is_parameterless_)
        {
            result += '(';
            result += params_.to_string();
            result += ')';
        }
        return result;
    }

    string Function::to_string() const
    {
        string result = header();
        if (is_defined())
            result += '\n' + body_->to_string();
        else
            result += ';';
        return result;
    }
}

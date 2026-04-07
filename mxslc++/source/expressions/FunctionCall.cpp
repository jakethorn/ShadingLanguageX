//
// Created by jaket on 29/11/2025.
//

#include "FunctionCall.h"

#include "runtime/Function.h"
#include "CompileError.h"
#include "runtime/Runtime.h"
#include "runtime/Scope.h"
#include "statements/Statement.h"
#include "utils/instantiate_template_types_utils.h"
#include "runtime/TypeInfo.h"
#include "runtime/Variable.h"

ExprPtr FunctionCall::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    Token name = token_.instantiate_template_types(template_type);
    TypeInfoPtr _template_type = ::instantiate_template_types(template_type_, template_type);
    ArgumentList args = args_.instantiate_template_types(template_type);
    return std::make_unique<FunctionCall>(runtime_, std::move(name), std::move(_template_type), std::move(args));
}

void FunctionCall::init_subexpressions(const vector<TypeInfoPtr>& types)
{
    if (arguments_are_initialized())
        return;

    const Scope& scope = runtime_.scope();

    size_t initialized_arg_count = initialized_arg_count_;
    while (initialized_arg_count < args_.size())
    {
        vector<FuncPtr> matching_funcs = scope.get_functions(types, token_, template_type_, args_);

        if (matching_funcs.empty())
            throw CompileError{token_, "No matching functions: " + token_.lexeme()};

        const size_t prev_initialized_arg_count = initialized_arg_count;
        initialized_arg_count = try_init_arguments(matching_funcs);

        if (initialized_arg_count == prev_initialized_arg_count)
        {
            // init failed, try to init with a default function...
            FuncPtr default_func = scope.get_function(types, token_, template_type_, args_);
            initialized_arg_count = try_init_arguments({std::move(default_func)});

            if (initialized_arg_count == prev_initialized_arg_count)
                throw CompileError{token_, "Ambiguous function call: " + token_.lexeme()};
        }
    }
}

void FunctionCall::init_impl(const vector<TypeInfoPtr>& types)
{
    const Scope& scope = runtime_.scope();
    func_ = scope.get_function(types, token_, template_type_, args_);
}

TypeInfoPtr FunctionCall::type_impl() const
{
    return func_->type();
}

ValuePtr FunctionCall::evaluate_impl() const
{
    if (func_->is_inline())
    {
        runtime_.enter_inline_scope();
        evaluate_arguments();
        for (const StmtPtr& stmt : func_->body())
            stmt->execute();
        ValuePtr return_value = evaluate_return();
        runtime_.exit_scope();
        return return_value;
    }
    else
    {
        return runtime_.serializer().write_node(func_, args_);
    }
}

namespace
{
    vector<TypeInfoPtr> get_parameter_types(const vector<FuncPtr>& funcs, const Argument& arg)
    {
        vector<TypeInfoPtr> types;

        for (const FuncPtr& func : funcs)
        {
            const TypeInfoPtr& type = func->parameters()[arg].type();
            if (not type->is_in(types))
                types.push_back(type);
        }

        return types;
    }
}

bool FunctionCall::arguments_are_initialized()
{
    bool are = true;
    for (const Argument& arg : args_)
    {
        if (arg.is_initialized())
        {
            arg.init(arg.type());
            ++initialized_arg_count_;
        }
        else
        {
            are = false;
        }
    }

    return are;
}

size_t FunctionCall::try_init_arguments(const vector<FuncPtr>& funcs)
{
    size_t initialized_arg_count = 0;
    for (const Argument& arg : args_)
    {
        if (not arg.is_initialized())
        {
            if (const vector<TypeInfoPtr> types = get_parameter_types(funcs, arg);
                arg.try_init(types))
            {
                ++initialized_arg_count_;
            }
        }

        if (arg.is_initialized())
        {
            ++initialized_arg_count;
        }
    }

    return initialized_arg_count;
}

void FunctionCall::evaluate_arguments() const
{
    for (const Parameter& param : func_->parameters())
    {
        ValuePtr val;
        if (const Argument* arg = args_[param])
        {
            val = arg->evaluate();
        }
        else if (param.has_default_value())
        {
            val = param.evaluate();
        }
        else
        {
            throw CompileError{token_, "Missing argument"s};
        }

        VarPtr var = std::make_shared<Variable>(ModifierList{}, param.name(), std::move(val));
        runtime_.scope().add_variable(std::move(var));
    }
}

ValuePtr FunctionCall::evaluate_return() const
{
    func_->return_expr()->init(func_->type());
    return func_->return_expr()->evaluate();
}

//
// Created by jaket on 16/04/2026.
//

#include "FunctionCall2.h"

#include <cassert>

#include "CompileError.h"
#include "runtime/Function2.h"
#include "runtime/Runtime.h"
#include "runtime/Scope.h"
#include "statements/Statement.h"
#include "utils/instantiate_template_types_utils.h"
#include "runtime/TypeInfo.h"
#include "runtime/Variable2.h"
#include "utils/error_utils.h"
#include "values/ValueFactory.h"

ExprPtr FunctionCall2::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    Token name = token_.instantiate_template_types(template_type);
    TypeInfoPtr _template_type = ::instantiate_template_types(template_type_, template_type);
    ArgumentList args = args_.instantiate_template_types(template_type);
    return std::make_unique<FunctionCall2>(std::move(name), std::move(_template_type), std::move(args));
}

void FunctionCall2::init_subexpressions(const vector<TypeInfoPtr>& types)
{
    const Scope& scope = Runtime::get().scope();

    if (template_type_)
        template_type_ = scope.resolve_type(template_type_);

    if (arguments_are_initialized())
        return;

    size_t initialized_arg_count = initialized_arg_count_;
    while (initialized_arg_count < args_.size())
    {
        vector<FuncPtr2> matching_funcs = scope.get_functions(types, name(), template_type_, args_);
        assert(not matching_funcs.empty());

        const size_t prev_initialized_arg_count = initialized_arg_count;
        initialized_arg_count = try_init_arguments(matching_funcs);

        if (initialized_arg_count == prev_initialized_arg_count)
        {
            // init failed, try to init with a default function...
            FuncPtr2 default_func = scope.get_function(types, name(), template_type_, args_);
            initialized_arg_count = try_init_arguments({std::move(default_func)});

            if (initialized_arg_count == prev_initialized_arg_count)
                throw CompileError{ambiguous_overload_error(matching_funcs)};
        }
    }
}

void FunctionCall2::init_impl(const vector<TypeInfoPtr>& types)
{
    func_ = Runtime::get().scope().get_function(types, name(), template_type_, args_);

    for (const Argument& arg : args_)
        arg.validate(func_->parameters()[arg]);
}

TypeInfoPtr FunctionCall2::type_impl() const
{
    return func_->return_type();
}

VarPtr2 FunctionCall2::evaluate_impl() const
{
    if (func_->is_inline())
    {
        Runtime::get().enter_inline_scope();
        evaluate_arguments();
        VarPtr2 return_value = func_->invoke();
        update_out_arguments();
        Runtime::get().exit_scope();
        return return_value;
    }
    else
    {
        VarPtr2 value = Runtime::get().serializer().write_node(func_, args_);
        return value;
    }
}

namespace
{
    vector<TypeInfoPtr> get_parameter_types(const vector<FuncPtr2>& funcs, const Argument& arg)
    {
        vector<TypeInfoPtr> types;

        for (const FuncPtr2& func : funcs)
        {
            const TypeInfoPtr& type = func->parameters()[arg].type();
            if (not type->is_in(types))
                types.push_back(type);
        }

        return types;
    }
}

bool FunctionCall2::arguments_are_initialized()
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

size_t FunctionCall2::try_init_arguments(const vector<FuncPtr2>& funcs)
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

// inline only
void FunctionCall2::evaluate_arguments() const
{
    for (const Parameter& param : func_->parameters())
    {
        if (param.is_in())
        {
            const VarPtr2 arg_value = args_.evaluate(param);
            const VarPtr2 arg_value_copy = Variable2::create(param.type(), arg_value);
            arg_value_copy->add_to_scope(param.name());
        }
        else
        {
            const VarPtr2 default_value = param.has_default_value() ? param.evaluate() : ValueFactory::create_default_value(param.type());
            default_value->set_modifiers(param.modifiers().without(TokenType::Ref, TokenType::Out));
            default_value->add_to_scope(param.name());
        }
    }
}

// inline only
void FunctionCall2::update_out_arguments() const
{
    for (const Parameter& param : func_->parameters())
    {
        if (param.is_out())
        {
            const VarPtr2 nonlocal = args_.evaluate(param);
            const VarPtr2 local = Runtime::get().scope().get_variable(param.name());
            nonlocal->copy_value(local);
        }
    }
}

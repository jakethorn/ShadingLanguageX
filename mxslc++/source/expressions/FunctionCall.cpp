//
// Created by jaket on 29/11/2025.
//

#include "FunctionCall.h"

#include "runtime/Function.h"
#include "CompileError.h"
#include "runtime/Runtime.h"
#include "runtime/Scope.h"

ExprPtr FunctionCall::instantiate_templated_types(const Type& template_type) const
{
    Token name = token_.instantiate_template_type(template_type);
    optional<Type> _template_type = Type::instantiate_template_type(template_type_, template_type);
    ArgumentList args = args_.instantiate_templated_types(template_type);
    return std::make_unique<FunctionCall>(runtime_, std::move(name), std::move(_template_type), std::move(args));
}

void FunctionCall::init_child_expressions(const vector<Type>& types)
{
    const Scope& scope = runtime_.scope();

    while (initialised_args_count_ < args_.size())
    {
        vector<const Function*> matching_funcs = scope.get_functions(types, token_, template_type_, args_);

        if (matching_funcs.empty())
            throw CompileError{"No matching functions"s};

        size_t prev_initialised_args_count = initialised_args_count_;
        try_init_arguments(matching_funcs);

        if (initialised_args_count_ == prev_initialised_args_count)
            throw CompileError{token_, "Ambiguous function call"s};
    }
}

void FunctionCall::init_impl(const vector<Type>& types)
{
    const Scope& scope = runtime_.scope();
    function_ = &scope.get_function(types, token_.lexeme(), template_type_, args_);
}

const Type& FunctionCall::type_impl() const
{
    return function_->type();
}

ValuePtr FunctionCall::evaluate_impl() const
{
    return runtime_.serializer().write_function_call(*function_, args_);
}

namespace
{
    vector<Type> get_parameter_types(const vector<const Function*>& funcs, const Argument& arg)
    {
        vector<Type> types;
        types.reserve(funcs.size());

        for (const Function* func : funcs)
        {
            const ParameterList& params = func->parameters();
            const Parameter& param = params[arg];
            const Type& type = param.type();
            types.push_back(type);
        }

        return types;
    }
}

void FunctionCall::try_init_arguments(const vector<const Function*>& funcs)
{
    for (const Argument& arg : args_)
    {
        if (not arg.is_initialized())
        {
            if (const vector<Type> types = get_parameter_types(funcs, arg);
                arg.try_init(types))
            {
                initialised_args_count_++;
            }
        }
    }
}

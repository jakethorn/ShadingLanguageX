//
// Created by jaket on 29/11/2025.
//

#include "FunctionCall.h"

//#include <cassert>
//
//#include "runtime/Function.h"
//#include "CompileError.h"
//#include "runtime/Runtime.h"
//#include "runtime/Scope.h"
//#include "statements/Statement.h"
//#include "utils/instantiate_template_types_utils.h"
//#include "runtime/TypeInfo.h"
//#include "../runtime/variables/Variable.h"
//#include "utils/error_utils.h"
//
//ExprPtr FunctionCall::instantiate_template_types(const TypeInfoPtr& template_type) const
//{
//    Token name = token_.instantiate_template_types(template_type);
//    TypeInfoPtr _template_type = ::instantiate_template_types(template_type_, template_type);
//    ArgumentList args = args_.instantiate_template_types(template_type);
//    return std::make_unique<FunctionCall>(runtime_, std::move(name), std::move(_template_type), std::move(args));
//}
//
//void FunctionCall::init_subexpressions(const vector<TypeInfoPtr>& types)
//{
//    const Scope& scope = runtime_.scope();
//    if (template_type_)
//        template_type_ = scope.resolve_type(template_type_);
//
//    if (arguments_are_initialized())
//        return;
//
//    size_t initialized_arg_count = initialized_arg_count_;
//    while (initialized_arg_count < args_.size())
//    {
//        vector<FuncPtr> matching_funcs = scope.get_functions(types, name(), template_type_, args_);
//        assert(not matching_funcs.empty());
//
//        const size_t prev_initialized_arg_count = initialized_arg_count;
//        initialized_arg_count = try_init_arguments(matching_funcs);
//
//        if (initialized_arg_count == prev_initialized_arg_count)
//        {
//            // init failed, try to init with a default function...
//            FuncPtr default_func = scope.get_function(types, name(), template_type_, args_);
//            initialized_arg_count = try_init_arguments({std::move(default_func)});
//
//            if (initialized_arg_count == prev_initialized_arg_count)
//                throw CompileError{ambiguous_overload_error(matching_funcs)};
//        }
//    }
//}
//
//void FunctionCall::init_impl(const vector<TypeInfoPtr>& types)
//{
//    const Scope& scope = runtime_.scope();
//    func_ = scope.get_function(types, name(), template_type_, args_);
//
//    for (const Argument& arg : args_)
//        arg.validate(func_->parameters()[arg]);
//}
//
//TypeInfoPtr FunctionCall::type_impl() const
//{
//    return func_->type();
//}
//
//ValuePtr FunctionCall::evaluate_impl() const
//{
//    const unordered_map<string, ValuePtr> args = evaluate_arguments();
//
//    if (func_->is_inline())
//    {
//        runtime_.enter_inline_scope();
//        add_arguments_to_scope(args);
//        ValuePtr return_value = func_->invoke();
//        runtime_.exit_scope();
//        return return_value;
//    }
//    else
//    {
//        ValuePtr val = runtime_.serializer().write_node(func_, args);
//        set_out_arguments();
//        return val;
//    }
//}
//
//namespace
//{
//    vector<TypeInfoPtr> get_parameter_types(const vector<FuncPtr>& funcs, const Argument& arg)
//    {
//        vector<TypeInfoPtr> types;
//
//        for (const FuncPtr& func : funcs)
//        {
//            const TypeInfoPtr& type = func->parameters()[arg].type();
//            if (not type->is_in(types))
//                types.push_back(type);
//        }
//
//        return types;
//    }
//}
//
//bool FunctionCall::arguments_are_initialized()
//{
//    bool are = true;
//    for (const Argument& arg : args_)
//    {
//        if (arg.is_initialized())
//        {
//            arg.init(arg.type());
//            ++initialized_arg_count_;
//        }
//        else
//        {
//            are = false;
//        }
//    }
//
//    return are;
//}
//
//size_t FunctionCall::try_init_arguments(const vector<FuncPtr>& funcs)
//{
//    size_t initialized_arg_count = 0;
//    for (const Argument& arg : args_)
//    {
//        if (not arg.is_initialized())
//        {
//            if (const vector<TypeInfoPtr> types = get_parameter_types(funcs, arg);
//                arg.try_init(types))
//            {
//                ++initialized_arg_count_;
//            }
//        }
//
//        if (arg.is_initialized())
//        {
//            ++initialized_arg_count;
//        }
//    }
//
//    return initialized_arg_count;
//}
//
//// outline/inline
//unordered_map<string, ValuePtr> FunctionCall::evaluate_arguments() const
//{
//    unordered_map<string, ValuePtr> result;
//
//    // iterate params in reverse order so node inputs are written in the "correct" order
//    // e.g.,
//    // <input name="in1" ... />
//    // <input name="in2" ... />
//    const vector<const Parameter*> in_params = func_->in_parameters();
//    for (size_t i = in_params.size(); i > 0; --i)
//    {
//        const Parameter& param = *in_params[i-1];
//
//        ValuePtr value;
//        if (const Argument* arg = args_[param])
//        {
//            value = arg->evaluate();
//        }
//        else if (param.has_default_value())
//        {
//            value = param.evaluate();
//        }
//        else
//        {
//            throw CompileError{"Function call '" + func_->name() +  "' missing argument for parameter '" + param.name() + "'"};
//        }
//
//        result[param.name()] = std::move(value);
//    }
//
//    return result;
//}
//
//// inline
//void FunctionCall::add_arguments_to_scope(const unordered_map<string, ValuePtr>& args) const
//{
//    for (const auto& [name, value] : args)
//    {
//        VarPtr var = std::make_shared<Variable>(ModifierList{}, name, value);
//        runtime_.scope().add_variable(std::move(var));
//    }
//
//    // add references to arguments passed to out parameters
//    for (const Parameter* param : func_->out_parameters())
//    {
//        runtime_.scope().add_reference(param->name(), args_[*param]->expression()->variable());
//    }
//}
//
//// outline
//void FunctionCall::set_out_arguments() const
//{
//    for (const Parameter* param : func_->out_parameters())
//    {
//        const ExprPtr arg_expr = args_[*param]->expression();
//        const VarPtr out_var = runtime_.scope().get_variable(func_->nonlocal_name(*param));
//
//        // temporarily disable const/mutable so we can assign the value to the out argument
//        const bool is_const = arg_expr->variable()->is_const();
//        const bool is_mutable = arg_expr->variable()->is_mutable();
//        arg_expr->variable()->set_const(false);
//        arg_expr->variable()->set_mutable(true);
//
//        arg_expr->assign(out_var->value());
//
//        arg_expr->variable()->set_const(is_const);
//        arg_expr->variable()->set_mutable(is_mutable);
//    }
//}

//
// Created by jaket on 01/12/2025.
//

#include "Function.h"

//#include <cassert>
//
//#include "CompileError.h"
//#include "Runtime.h"
//#include "TypeInfo.h"
//#include "variables/Variable.h"
//#include "statements/Statement.h"
//#include "expressions/Expression.h"
//#include "mtlx/mtlx_utils.h"
//#include "values/Value.h"
//#include "values/ValueFactory.h"
//
//Function::Function(
//    const Runtime& runtime,
//    ModifierList mods,
//    TypeInfoPtr type,
//    string name,
//    TypeInfoPtr template_type,
//    ParameterList params,
//    vector<string> output_names
//) : runtime_{runtime},
//    mods_{std::move(mods)},
//    type_{std::move(type)},
//    name_{std::move(name)},
//    template_type_{std::move(template_type)},
//    params_{std::move(params)},
//    output_names_({std::move(output_names)})
//{
//
//}
//
//Function::Function(
//    const Runtime& runtime,
//    ModifierList mods,
//    TypeInfoPtr type,
//    string name,
//    TypeInfoPtr template_type,
//    ParameterList params,
//    StmtPtr body,
//    ExprPtr return_expr
//) : runtime_{runtime},
//    mods_{std::move(mods)},
//    type_{std::move(type)},
//    name_{std::move(name)},
//    template_type_{std::move(template_type)},
//    params_{std::move(params)},
//    body_{std::move(body)},
//    return_expr_{std::move(return_expr)}
//{
//    mods_.validate(TokenType::Inline, TokenType::Default);
//
//    if (type_ == TypeInfo::Void and return_expr_ != nullptr)
//        throw CompileError{"Void function '" + name_ + "' has a return statement"s};
//    if (type_ != TypeInfo::Void and return_expr_ == nullptr)
//        throw CompileError{"Non-void function '" + name_ + "' does not have a return statement"s};
//}
//
//Function::Function(Function&& other) noexcept
//    : runtime_{other.runtime_},
//    mods_{std::move(other.mods_)},
//    type_{std::move(other.type_)},
//    name_{std::move(other.name_)},
//    template_type_{std::move(other.template_type_)},
//    params_{std::move(other.params_)},
//    body_{std::move(other.body_)},
//    return_expr_{std::move(other.return_expr_)},
//    output_names_{std::move(other.output_names_)}
//{
//}
//
//Function::~Function() = default;
//
//bool Function::is_void() const
//{
//    return type_ == TypeInfo::Void;
//}
//
//size_t Function::min_arity() const
//{
//    size_t arity = 0;
//    for (const Parameter& param : params_)
//    {
//        if (not param.has_default_value())
//            ++arity;
//    }
//
//    return arity;
//}
//
//vector<const Parameter*> Function::in_parameters() const
//{
//    vector<const Parameter*> result;
//    for (const Parameter& param : params_)
//    {
//        if (not param.is_out())
//            result.push_back(&param);
//    }
//
//    return result;
//}
//
//vector<const Parameter*> Function::out_parameters() const
//{
//    vector<const Parameter*> result;
//    for (const Parameter& param : params_)
//    {
//        if (param.is_out())
//            result.push_back(&param);
//    }
//
//    return result;
//}
//
//// outline/inline
//void Function::init()
//{
//    if (type_ == TypeInfo::Void)
//        type_ = TypeInfo::resolved_void();
//    else
//        type_ = runtime_.scope().resolve_type(type_);
//
//    if (output_names_.empty())
//    {
//        for (size_t i = 0; i < type_->field_count(); ++i)
//            output_names_.push_back(port_name("out"s, i));
//    }
//
//    if (template_type_)
//        template_type_ = runtime_.scope().resolve_type(template_type_);
//
//    params_.init();
//
//    is_initialized_ = true;
//}
//
//// outline/inline
//ValuePtr Function::invoke() const
//{
//    body_->execute();
//    return evaluate_return();
//}
//
//// outline/inline
//ValuePtr Function::evaluate_return() const
//{
//    if (return_expr_ != nullptr)
//    {
//        assert(not is_void());
//        return_expr_->init(type_);
//        return return_expr_->evaluate();
//    }
//    else
//    {
//        assert(is_void());
//        return nullptr;
//    }
//}
//
//void Function::add_nonlocal_input(const string& name, const ConstVarPtr& var)
//{
//    nonlocal_inputs_[name] = var;
//}
//
//void Function::add_nonlocal_output(const string& name, const VarPtr& var)
//{
//    nonlocal_outputs_[name] = var;
//}
//
//string Function::nonlocal_name(const Parameter& param) const
//{
//    return name_ + "_" + (template_type_ ? template_type_->name() : ""s) + "__" + param.name();
//}
//
//string Function::str() const
//{
//    string result;
//    result += mods_.str();
//    result += type_->str();
//    result += " " + name_;
//    if (template_type_)
//        result += "<" + template_type_->str() + ">";
//    result += "(";
//    result += params_.str();
//    result += ")";
//    return result;
//}

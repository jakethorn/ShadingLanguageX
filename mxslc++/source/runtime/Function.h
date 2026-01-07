//
// Created by jaket on 12/11/2025.
//

#ifndef FENNEC_FUNCTION_H
#define FENNEC_FUNCTION_H

#include "utils/common.h"
#include "utils/template_utils.h"
#include "Type.h"
#include "Token.h"
#include "ParameterList.h"

class Function
{
public:
    Function(vector<string> modifiers, Type type, Token name, optional<Type> template_type, ParameterList params, vector<StmtPtr> body, ExprPtr return_expr);

    Function(Function&& other) noexcept;
    Function& operator=(Function&& other) noexcept;

    Function(const Function&) = delete;
    Function& operator=(const Function&) = delete;

    ~Function();

    [[nodiscard]] bool is_inline() const { return contains(modifiers_, "inline"s); }
    [[nodiscard]] const Type& type() const { return type_; }
    [[nodiscard]] const string& name() const { return name_.lexeme(); }
    [[nodiscard]] bool has_template_type() const { return template_type_.has_value(); }
    [[nodiscard]] const Type& template_type() const { return template_type_.value(); }
    [[nodiscard]] size_t arity() const { return params_.size(); }
    [[nodiscard]] const ParameterList& parameters() const { return params_; }
    [[nodiscard]] const vector<StmtPtr>& body() const { return body_; }
    [[nodiscard]] const ExprPtr& return_expr() const { return return_expr_; }
    [[nodiscard]] const Token& token() const { return name_; }

private:
    vector<string> modifiers_;
    Type type_;
    Token name_;
    optional<Type> template_type_;
    ParameterList params_;
    vector<StmtPtr> body_;
    ExprPtr return_expr_;
};

#endif //FENNEC_FUNCTION_H

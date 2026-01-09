//
// Created by jaket on 07/11/2025.
//

#ifndef FENNEC_PARAMETER_H
#define FENNEC_PARAMETER_H

#include "utils/common.h"
#include "utils/template_utils.h"
#include "Token.h"
#include "Type.h"

class Parameter
{
public:
    Parameter(vector<string> modifiers, Type type, Token name, size_t index);
    Parameter(vector<string> modifiers, Type type, Token name, ExprPtr expr, size_t index);

    Parameter(Parameter&&) noexcept;

    ~Parameter();

    [[nodiscard]] bool is_out() const { return contains(modifiers_, "out"s); }
    [[nodiscard]] const Type& type() const { return type_; }
    [[nodiscard]] const string& name() const { return name_.lexeme(); }
    [[nodiscard]] size_t index() const { return index_; }

    [[nodiscard]] Parameter instantiate_template_types(const Type& template_type) const;
    void init() const;

    [[nodiscard]] bool has_default_value() const { return expr_ != nullptr; }
    [[nodiscard]] ValuePtr evaluate() const;

private:
    vector<string> modifiers_;
    Type type_;
    Token name_;
    ExprPtr expr_;
    size_t index_;
};

#endif //FENNEC_PARAMETER_H

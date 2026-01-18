//
// Created by jaket on 07/11/2025.
//

#ifndef FENNEC_PARAMETER_H
#define FENNEC_PARAMETER_H

#include "ModifierList.h"
#include "utils/common.h"
#include "utils/template_utils.h"
#include "Token.h"

class Runtime;

class Parameter
{
public:
    Parameter(const Runtime& runtime, ModifierList mods, Token type, Token name, size_t index);
    Parameter(const Runtime& runtime, ModifierList mods, Token type, Token name, ExprPtr expr, size_t index);

    Parameter(Parameter&&) noexcept;

    ~Parameter();

    [[nodiscard]] bool is_out() const { return mods_.contains("out"s); }
    [[nodiscard]] const string& name() const { return name_.lexeme(); }
    [[nodiscard]] size_t index() const { return index_; }

    [[nodiscard]] Parameter instantiate_template_types(const TypeInfoPtr& template_type) const;
    void init() const;
    [[nodiscard]] TypeInfoPtr type() const;

    [[nodiscard]] bool has_default_value() const { return expr_ != nullptr; }
    [[nodiscard]] ValuePtr evaluate() const;

private:
    const Runtime& runtime_;
    ModifierList mods_;
    Token type_;
    Token name_;
    ExprPtr expr_;
    size_t index_;
};

#endif //FENNEC_PARAMETER_H

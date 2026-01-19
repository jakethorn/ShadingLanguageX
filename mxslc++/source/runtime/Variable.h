//
// Created by jaket on 12/11/2025.
//

#ifndef FENNEC_VARIABLE_H
#define FENNEC_VARIABLE_H

#include "ModifierList.h"
#include "utils/common.h"
#include "Token.h"

class Variable
{
public:
    Variable() = default;
    Variable(ModifierList mods, TypeInfoPtr type, Token name);
    Variable(ModifierList mods, TypeInfoPtr type, Token name, ValuePtr value);

    [[nodiscard]] bool is_const() const { return mods_.contains("const"s); }
    [[nodiscard]] bool is_mutable() const { return mods_.contains("mutable"s); }
    [[nodiscard]] bool is_global() const { return mods_.contains("global"s); }
    [[nodiscard]] TypeInfoPtr type() const { return type_; }
    [[nodiscard]] const string& name() const { return name_.lexeme(); }
    [[nodiscard]] ValuePtr value() const { return value_; }
    [[nodiscard]] const Token& token() const { return name_; }

private:
    ModifierList mods_;
    TypeInfoPtr type_;
    Token name_;
    ValuePtr value_;
};

#endif //FENNEC_VARIABLE_H

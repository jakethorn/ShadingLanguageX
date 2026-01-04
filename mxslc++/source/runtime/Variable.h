//
// Created by jaket on 12/11/2025.
//

#ifndef FENNEC_VARIABLE_H
#define FENNEC_VARIABLE_H

#include "utils/common.h"
#include "utils/template_utils.h"
#include "Token.h"
#include "Type.h"

class Variable
{
public:
    Variable(vector<string> modifiers, Type type, Token name);
    Variable(vector<string> modifiers, Type type, Token name, ValuePtr value);

    [[nodiscard]] bool is_const() const { return contains(modifiers_, "const"s); }
    [[nodiscard]] bool is_mutable() const { return contains(modifiers_, "mutable"s); }
    [[nodiscard]] bool is_global() const { return contains(modifiers_, "global"s); }
    [[nodiscard]] const Type& type() const { return type_; }
    [[nodiscard]] const string& name() const { return name_.lexeme(); }
    [[nodiscard]] ValuePtr value() const { return value_; }
    [[nodiscard]] const Token& token() const { return name_; }

private:
    vector<string> modifiers_;
    Type type_;
    Token name_;
    ValuePtr value_;
};

#endif //FENNEC_VARIABLE_H

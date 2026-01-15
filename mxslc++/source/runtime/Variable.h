//
// Created by jaket on 12/11/2025.
//

#ifndef FENNEC_VARIABLE_H
#define FENNEC_VARIABLE_H

#include "utils/common.h"
#include "utils/template_utils.h"
#include "Token.h"
#include "Type.h"

struct VariableDeclaration
{
    vector<string> mods;
    Type type;
    Token name;

    VariableDeclaration(vector<string> mods, Type type, Token name)
        : mods{std::move(mods)}, type{std::move(type)}, name{std::move(name)} { }

    [[nodiscard]] VariableDeclaration instantiate_template_types(const Type& template_type) const;
};

class Variable
{
public:
    Variable() = default;
    Variable(vector<string> mods, Type type, Token name);
    Variable(vector<string> mods, Type type, Token name, ValuePtr value);
    Variable(VariableDeclaration decl);
    Variable(VariableDeclaration decl, ValuePtr value);

    [[nodiscard]] bool is_const() const { return contains(mods_, "const"s); }
    [[nodiscard]] bool is_mutable() const { return contains(mods_, "mutable"s); }
    [[nodiscard]] bool is_global() const { return contains(mods_, "global"s); }
    [[nodiscard]] const Type& type() const { return type_; }
    [[nodiscard]] const string& name() const { return name_.lexeme(); }
    [[nodiscard]] ValuePtr value() const { return value_; }
    [[nodiscard]] const Token& token() const { return name_; }

private:
    vector<string> mods_;
    Type type_;
    Token name_;
    ValuePtr value_;
};

#endif //FENNEC_VARIABLE_H

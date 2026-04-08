//
// Created by jaket on 12/11/2025.
//

#ifndef FENNEC_VARIABLE_H
#define FENNEC_VARIABLE_H

#include "ModifierList.h"
#include "utils/common.h"
#include "Token.h"

class IVariable
{
public:
    virtual ~IVariable() = default;
    [[nodiscard]] virtual bool is_const() const = 0;
    [[nodiscard]] virtual bool is_mutable() const = 0;
    [[nodiscard]] virtual bool is_global() const = 0;
    [[nodiscard]] virtual TypeInfoPtr type() const = 0;
    [[nodiscard]] virtual const string& name() const = 0;
    [[nodiscard]] virtual ValuePtr value() const = 0;
    [[nodiscard]] virtual const Token& name_token() const = 0;

    void virtual set_value(const ValuePtr& val) = 0;
};

class Variable final : public IVariable
{
public:
    Variable() = default;
    Variable(ModifierList mods, Token name, ValuePtr val);

    [[nodiscard]] bool is_const() const override { return mods_.contains("const"s); }
    [[nodiscard]] bool is_mutable() const override { return mods_.contains("mutable"s); }
    [[nodiscard]] bool is_global() const override { return mods_.contains("global"s); }
    [[nodiscard]] TypeInfoPtr type() const override;
    [[nodiscard]] const string& name() const override { return name_.lexeme(); }
    [[nodiscard]] ValuePtr value() const override { return val_; }
    [[nodiscard]] const Token& name_token() const override { return name_; }

    void set_value(const ValuePtr& val) override { val_ = val; }

private:
    ModifierList mods_;
    Token name_;
    ValuePtr val_;
};

class SubVariable final : public IVariable
{
public:
    SubVariable() = default;
    SubVariable(VarPtr owner, size_t index);
    SubVariable(VarPtr owner, Token name);

    [[nodiscard]] bool is_const() const override;
    [[nodiscard]] bool is_mutable() const override;
    [[nodiscard]] bool is_global() const override;
    [[nodiscard]] TypeInfoPtr type() const override;
    [[nodiscard]] const string& name() const override;
    [[nodiscard]] ValuePtr value() const override;
    [[nodiscard]] const Token& name_token() const override;

    void set_value(const ValuePtr& val) override;

private:
    VarPtr owner_;
    Token name_;
    size_t index_;
    string qualified_name_;
};

VarPtr get_subvariable(VarPtr owner, size_t index);
VarPtr get_subvariable(VarPtr owner, Token name);

#endif //FENNEC_VARIABLE_H

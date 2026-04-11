//
// Created by jaket on 12/11/2025.
//

#ifndef FENNEC_VARIABLE_H
#define FENNEC_VARIABLE_H

#include "ModifierList.h"
#include "utils/common.h"

class FieldInfo;

class IVariable
{
public:
    virtual ~IVariable() = default;

    virtual bool is_const() const = 0;
    virtual bool is_mutable() const = 0;
    virtual bool is_global() const = 0;
    virtual TypeInfoPtr type() const = 0;
    virtual const string& name() const = 0;
    virtual ValuePtr value() const = 0;

    void virtual set_value(const ValuePtr& val) = 0;
};

class Variable final : public IVariable
{
public:
    Variable(ModifierList mods, string name, ValuePtr val);

    bool is_const() const override { return mods_.contains(TokenType::Const); }
    bool is_mutable() const override { return mods_.contains(TokenType::Mutable); }
    bool is_global() const override { return mods_.contains(TokenType::Global); }
    TypeInfoPtr type() const override;
    const string& name() const override { return name_; }
    ValuePtr value() const override { return val_; }

    void set_value(const ValuePtr& val) override { val_ = val; }

private:
    ModifierList mods_;
    string name_;
    ValuePtr val_;
};

class SubVariable final : public IVariable
{
public:
    SubVariable(VarPtr owner, size_t index);
    SubVariable(VarPtr owner, const string& name);

    bool is_const() const override;
    bool is_mutable() const override;
    bool is_global() const override;
    TypeInfoPtr type() const override;
    const FieldInfo& field() const;
    const string& name() const override;
    ValuePtr value() const override;

    void set_value(const ValuePtr& val) override;

private:
    VarPtr owner_;
    size_t index_;
    string name_;
};

VarPtr get_subvariable(VarPtr owner, size_t index);
VarPtr get_subvariable(VarPtr owner, const string& name);

#endif //FENNEC_VARIABLE_H

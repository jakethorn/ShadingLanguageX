//
// Created by jaket on 12/11/2025.
//

#ifndef FENNEC_VARIABLE_H
#define FENNEC_VARIABLE_H

#include "ModifierList.h"
#include "utils/common.h"

class FieldInfo;

/*
 * Variable Interface
 */

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

    virtual void set_value(const ValuePtr& val) = 0;

    virtual bool is_child() const { return false; }
    virtual VarPtr parent() const { return nullptr; }

    virtual bool is_reference() const { return false; }
    virtual VarPtr dereference() const { return nullptr; }
};

/*
 * Variable
 */

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

/*
 * SubVariable
 */

class SubVariable final : public IVariable
{
public:
    SubVariable(VarPtr parent, size_t index);
    SubVariable(VarPtr parent, const string& name);

    bool is_const() const override;
    bool is_mutable() const override;
    bool is_global() const override;
    TypeInfoPtr type() const override;
    const FieldInfo& field() const;
    const string& name() const override;
    ValuePtr value() const override;

    void set_value(const ValuePtr& val) override;

    bool is_child() const override { return true; }
    VarPtr parent() const override { return parent_; }

private:
    VarPtr parent_;
    size_t index_;
    string name_;
};

VarPtr get_subvariable(VarPtr parent, size_t index);
VarPtr get_subvariable(VarPtr parent, const string& name);

/*
 * VariableReference
 */

class VariableReference final : public IVariable
{
public:
    VariableReference(string name, VarPtr var) : name_{std::move(name)}, var_{std::move(var)} { }

    bool is_const() const override { return var_->is_const(); }
    bool is_mutable() const override { return var_->is_mutable(); }
    bool is_global() const override { return var_->is_global(); }
    TypeInfoPtr type() const override { return var_->type(); }
    const string& name() const override { return name_; }
    ValuePtr value() const override { return var_->value(); }

    void set_value(const ValuePtr& val) override { var_->set_value(val); }

    bool is_reference() const override { return true; }
    VarPtr dereference() const override { return var_; }

private:
    string name_;
    VarPtr var_;
};

#endif //FENNEC_VARIABLE_H

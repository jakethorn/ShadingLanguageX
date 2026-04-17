//
// Created by jaket on 14/04/2026.
//

#ifndef MXSLC_INVARIABLE_H
#define MXSLC_INVARIABLE_H

#include "IVariable.h"
#include "runtime/ModifierList.h"

class InVariable final : public IVariable
{
public:
    InVariable(const Runtime& runtime, ModifierList mods, string name, VarPtr var)
        : IVariable{runtime}, mods_{std::move(mods)}, name_{std::move(name)}, ref_{std::move(var)} { }

    bool is_const() const override { return mods_.contains(TokenType::Const); }
    bool is_mutable() const override { return mods_.contains(TokenType::Mutable); }
    bool is_global() const override { return false; }
    TypeInfoPtr type() const override { return ref_->type(); }
    const string& name() const override { return name_; }

    ValuePtr value() override;
    ValuePtr subvalue(size_t index) override;

    void set_value(ValuePtr value) override;
    void set_subvalue(size_t index, ValuePtr value) override;

    void set_const(const bool cnt) override { ref_->set_const(cnt); }
    void set_mutable(const bool mut) override { ref_->set_mutable(mut); }

    bool is_reference() const override { return true; }
    VarPtr dereference() const override { return ref_; }

    ValuePtr raw_value() const override { return value_ ? value_ : ref_->raw_value(); }

private:
    ModifierList mods_;
    string name_;
    VarPtr ref_;
    ValuePtr value_ = nullptr;
};

#endif //MXSLC_INVARIABLE_H

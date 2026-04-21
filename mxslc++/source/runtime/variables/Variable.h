//
// Created by jaket on 12/11/2025.
//

#ifndef FENNEC_VARIABLE_H
#define FENNEC_VARIABLE_H

//#include "IVariable.h"
//#include "runtime/ModifierList.h"
//#include "utils/common.h"
//
//class Variable final : public IVariable
//{
//public:
//    Variable(const Runtime& runtime, ModifierList mods, string name, ValuePtr value);
//
//    bool is_const() const override { return mods_.contains(TokenType::Const); }
//    bool is_mutable() const override { return mods_.contains(TokenType::Mutable); }
//    bool is_global() const override { return mods_.contains(TokenType::Global); }
//    TypeInfoPtr type() const override;
//    const string& name() const override { return name_; }
//
//    ValuePtr value() override;
//    ValuePtr subvalue(size_t index) override;
//
//    void set_value(ValuePtr value) override;
//    void set_subvalue(size_t index, ValuePtr value) override;
//
//    void set_const(bool cnt) override;
//    void set_mutable(bool mut) override;
//
//    ValuePtr raw_value() const override { return value_; }
//
//private:
//    ModifierList mods_;
//    string name_;
//    ValuePtr value_ = nullptr;
//};

#endif //FENNEC_VARIABLE_H

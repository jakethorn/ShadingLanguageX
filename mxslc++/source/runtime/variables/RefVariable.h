//
// Created by jaket on 14/04/2026.
//

#ifndef MXSLC_REFVARIABLE_H
#define MXSLC_REFVARIABLE_H

//#include "IVariable.h"
//
//class RefVariable final : public IVariable
//{
//public:
//    RefVariable(const Runtime& runtime, string name, VarPtr var)
//        : IVariable{runtime}, name_{std::move(name)}, ref_{std::move(var)} { }
//
//    bool is_const() const override { return ref_->is_const(); }
//    bool is_mutable() const override { return ref_->is_mutable(); }
//    bool is_global() const override { return ref_->is_global(); }
//    TypeInfoPtr type() const override { return ref_->type(); }
//    const string& name() const override { return name_; }
//
//    ValuePtr value() override { return ref_->value(); }
//    ValuePtr subvalue(const size_t index) override { return ref_->subvalue(index); }
//
//    void set_value(ValuePtr val) override { ref_->set_value(std::move(val)); }
//    void set_subvalue(const size_t index, ValuePtr val) override { ref_->set_subvalue(index, std::move(val)); }
//
//    void set_const(const bool cnt) override { ref_->set_const(cnt); }
//    void set_mutable(const bool mut) override { ref_->set_mutable(mut); }
//
//    bool is_reference() const override { return true; }
//    VarPtr dereference() const override { return ref_; }
//
//    ValuePtr raw_value() const override { return ref_->raw_value(); }
//
//private:
//    string name_;
//    VarPtr ref_;
//};

#endif //MXSLC_REFVARIABLE_H

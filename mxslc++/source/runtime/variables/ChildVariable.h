//
// Created by jaket on 14/04/2026.
//

#ifndef MXSLC_CHILDVARIABLE_H
#define MXSLC_CHILDVARIABLE_H

//#include "IVariable.h"
//
//class FieldInfo;
//
//class ChildVariable final : public IVariable
//{
//public:
//    ChildVariable(const Runtime& runtime, VarPtr parent, size_t index);
//    ChildVariable(const Runtime& runtime, VarPtr parent, const string& name);
//
//    bool is_const() const override;
//    bool is_mutable() const override;
//    bool is_global() const override;
//    TypeInfoPtr type() const override;
//    const string& name() const override;
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
//    bool is_child() const override { return true; }
//    ConstVarPtr parent() const override { return parent_; }
//
//    ValuePtr raw_value() const override;
//
//private:
//    const FieldInfo& field() const;
//
//    VarPtr parent_;
//    size_t index_;
//    string name_;
//};

#endif //MXSLC_CHILDVARIABLE_H

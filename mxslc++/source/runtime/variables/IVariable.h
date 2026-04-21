//
// Created by jaket on 14/04/2026.
//

#ifndef MXSLC_IVARIABLE_H
#define MXSLC_IVARIABLE_H

//#include "utils/common.h"
//
//class Runtime;
//
//class IVariable : public std::enable_shared_from_this<IVariable>
//{
//public:
//    explicit IVariable(const Runtime& runtime) : runtime_{runtime} { }
//    virtual ~IVariable() = default;
//
//    virtual bool is_const() const = 0;
//    virtual bool is_mutable() const = 0;
//    virtual bool is_global() const = 0;
//    virtual TypeInfoPtr type() const = 0;
//    virtual const string& name() const = 0;
//
//    virtual ValuePtr value() = 0;
//    virtual ValuePtr subvalue(size_t index) = 0;
//
//    virtual void set_value(ValuePtr value) = 0;
//    virtual void set_subvalue(size_t index, ValuePtr value) = 0;
//
//    virtual void set_const(bool cnt) = 0;
//    virtual void set_mutable(bool mut) = 0;
//
//    virtual bool is_child() const { return false; }
//    virtual ConstVarPtr parent() const { return nullptr; }
//
//    virtual bool is_reference() const { return false; }
//    virtual VarPtr dereference() const { return nullptr; }
//
//    virtual ValuePtr raw_value() const = 0;
//
//    ConstVarPtr self() const { return shared_from_this(); }
//    VarPtr self() { return shared_from_this(); }
//    VarPtr child(size_t index);
//    VarPtr child(const string& name);
//
//protected:
//    void validate(const ValuePtr& value) const;
//    bool is_inline() const;
//
//    const Runtime& runtime_;
//};

#endif //MXSLC_IVARIABLE_H

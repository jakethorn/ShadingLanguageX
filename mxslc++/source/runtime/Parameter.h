//
// Created by jaket on 07/11/2025.
//

#ifndef FENNEC_PARAMETER_H
#define FENNEC_PARAMETER_H

#include "ModifierList.h"
#include "utils/common.h"

class Runtime;

class Parameter
{
public:
    Parameter(const Runtime& runtime, ModifierList mods, TypeInfoPtr type, string name, size_t index);
    Parameter(const Runtime& runtime, ModifierList mods, TypeInfoPtr type, string name, ExprPtr expr, size_t index);

    Parameter(Parameter&&) noexcept;

    ~Parameter();

    bool is_const() const { return mods_.contains(TokenType::Const); }
    bool is_mutable() const { return mods_.contains(TokenType::Mutable); }
    bool is_out() const { return mods_.contains(TokenType::Out); }
    const string& name() const { return name_; }
    size_t index() const { return index_; }

    Parameter instantiate_template_types(const TypeInfoPtr& template_type) const;
    void init();
    TypeInfoPtr type() const;

    bool has_default_value() const { return expr_ != nullptr; }
    ValuePtr evaluate() const;

    string str() const;

private:
    const Runtime& runtime_;
    ModifierList mods_;
    TypeInfoPtr type_;
    string name_;
    ExprPtr expr_;
    size_t index_;
};

#endif //FENNEC_PARAMETER_H

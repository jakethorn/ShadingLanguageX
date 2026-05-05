//
// Created by jaket on 07/11/2025.
//

#ifndef FENNEC_PARAMETER_H
#define FENNEC_PARAMETER_H

#include "AttributeList.h"
#include "ModifierList.h"
#include "utils/common.h"

class Parameter
{
public:
    Parameter(AttributeList attrs, ModifierList mods, TypePtr type, string name, ExprPtr expr, size_t index);

    Parameter(Parameter&&) noexcept;

    ~Parameter();

    const AttributeList& attributes() const { return attrs_; }
    const ModifierList& modifiers() const { return mods_; }
    bool is_const() const { return mods_.contains(TokenType::Const); }
    bool is_mutable() const { return mods_.contains(TokenType::Mutable); }
    bool is_in() const { return not mods_.contains(TokenType::Out); }
    bool is_out() const { return mods_.contains(TokenType::Ref) or mods_.contains(TokenType::Out); }
    const string& name() const { return name_; }
    size_t index() const { return index_; }

    Parameter instantiate_template_types(const TypePtr& template_type) const;
    void init();
    TypePtr type() const;

    bool has_default_value() const { return expr_ != nullptr; }
    VarPtr evaluate() const;

    string str() const;

private:
    AttributeList attrs_;
    ModifierList mods_;
    TypePtr type_;
    string name_;
    ExprPtr expr_;
    size_t index_;
};

#endif //FENNEC_PARAMETER_H

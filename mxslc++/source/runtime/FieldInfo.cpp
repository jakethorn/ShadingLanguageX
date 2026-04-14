//
// Created by jaket on 18/01/2026.
//

#include "FieldInfo.h"
#include "TypeInfo.h"
#include "expressions/Expression.h"

FieldInfo FieldInfo::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    TypeInfoPtr type = type_->instantiate_template_types(template_type);
    ExprPtr initializer = initializer_->instantiate_template_types(template_type);
    return FieldInfo{mods_, std::move(type), name_, std::move(initializer)};
}

void FieldInfo::set_const(const bool cnt) const
{
    if (cnt)
        mods_.add(TokenType::Const);
    else
        mods_.remove(TokenType::Const);
}

void FieldInfo::set_mutable(const bool mut) const
{
    if (mut)
        mods_.add(TokenType::Mutable);
    else
        mods_.remove(TokenType::Mutable);
}

string FieldInfo::str() const
{
    string result = type_->str();
    if (has_name())
        result += " " + name();
    return result;
}

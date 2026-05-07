//
// Created by jaket on 18/01/2026.
//

#include "Field.h"
#include "Type.h"
#include "expressions/Expression.h"

Field Field::instantiate_template_types(const TypePtr& template_type) const
{
    TypePtr type = type_->instantiate_template_types(template_type);
    return Field{mods_, std::move(type), name_};
}

string Field::str() const
{
    string result = type_->str();
    if (has_name())
        result += " " + name();
    return result;
}

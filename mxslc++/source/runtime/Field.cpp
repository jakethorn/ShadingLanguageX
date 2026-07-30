//
// Created by jaket on 18/01/2026.
//

#include "runtime/Field.h"
#include "runtime/Type.h"
#include "expressions/Expression.h"

namespace mxslc::runtime
{
    Field Field::monomorphize(const TypePtr& template_type) const
    {
        TypePtr type = type_->monomorphize(template_type);
        return Field{mods_, std::move(type), name_};
    }

    string Field::to_string() const
    {
        string result = type_->str();
        if (has_name())
            result += " " + name();
        return result;
    }
}

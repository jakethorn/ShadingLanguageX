//
// Created by jaket on 10/07/2026.
//

#include "runtime/utils/type_utils.h"

#include "runtime/interface.h"
#include "runtime/Runtime.h"
#include "runtime/Scope.h"
#include "runtime/Type.h"
#include "runtime/variables/Variable.h"

namespace mxslc::type_utils
{
    TypePtr type_of(const vector<VarPtr>& children)
    {
        vector<TypePtr> fields;
        fields.reserve(children.size());
        for (const VarPtr& child : children)
            fields.push_back(child->type());

        return Runtime::get().scope().resolve_type(
            create_type(std::move(fields))
        );
    }

    bool contains_auto(const TypePtr& type)
    {
        if (type->is_auto())
            return true;
        if (type->has_fields())
        {
            for (const Field& field : type->fields())
            {
                if (contains_auto(field.type()))
                    return true;
            }
        }
        return false;
    }

    TypePtr replace_auto(const TypePtr& original_type, const TypePtr& replacement_type)
    {
        if (original_type->is_auto())
            return replacement_type;
        if (original_type->has_fields())
        {
            vector<Field> fields;
            fields.reserve(original_type->field_count());
            for (size_t i = 0; i < original_type->field_count(); ++i)
            {
                Field original_field = original_type->field(i);
                fields.emplace_back(
                    original_field.modifiers(),
                    replace_auto(original_field.type(), replacement_type->field_type(i)),
                    original_field.name()
                );
            }
            return create_type(original_type->name(), std::move(fields));
        }
        return original_type;
    }

    string to_string(const vector<TypePtr>& types)
    {
        if (types.empty())
            return "";

        if (types.size() == 1)
            return types[0]->to_string();

        string result = "(";
        for (size_t i = 0; i < types.size(); ++i)
        {
            result += types[i]->to_string();
            if (i < types.size() - 1)
                result += ", ";
        }
        result += ")";
        return result;
    }
}

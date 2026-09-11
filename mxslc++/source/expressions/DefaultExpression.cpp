//
// Created by jaket on 11/09/2026.
//

#include "expressions/DefaultExpression.h"

#include "expressions/interface.h"
#include "runtime/interface.h"
#include "runtime/Scope.h"
#include "runtime/Type.h"
#include "runtime/utils/monomorphize.h"
#include "serialize/values/interface.h"
#include "serialize/values/StringValue.h"

namespace mxslc::expressions
{
    DefaultExpression::DefaultExpression(TypePtr type, Token token) : Expression{std::move(token)}, type_{std::move(type)}
    {

    }

    ExprPtr DefaultExpression::monomorphize(const TypePtr& template_type) const
    {
        TypePtr type = runtime_utils::monomorphize(type_, template_type );
        return create_expression<DefaultExpression>(std::move(type), token_);
    }

    void DefaultExpression::init_impl(const vector<TypePtr>& types)
    {
        if (type_)
            type_ = scope().resolve_type(type_);
        else if (types.size() == 1)
            type_ = types[0];
        else
            throw CompileError{"Ambiguous default expression"};
    }

    TypePtr DefaultExpression::type_impl() const
    {
        return type_;
    }

    namespace
    {
        VarPtr get_default_value(TypePtr type)
        {
            if (type->has_fields())
            {
                vector<VarPtr> children;
                children.reserve(type->fields().size());
                for (const auto& field : type->fields())
                    children.push_back(get_default_value(field.type()));
                return create_variable(std::move(type), children);
            }
            else
            {
                string value{""};

#define DEFAULT_VALUE(t, s) if (type->is<t>()) value = s;
                DEFAULT_VALUE(bool, "false")
                DEFAULT_VALUE(int, "0")
                DEFAULT_VALUE(float, "0.0")
                DEFAULT_VALUE(string, "")
                DEFAULT_VALUE(fs::path, "")
                DEFAULT_VALUE(mx::Vector2, "0.0, 0.0")
                DEFAULT_VALUE(mx::Vector3, "0.0, 0.0, 0.0")
                DEFAULT_VALUE(mx::Vector4, "0.0, 0.0, 0.0, 0.0")
                DEFAULT_VALUE(mx::Color3, "0.0, 0.0, 0.0")
                DEFAULT_VALUE(mx::Color4, "0.0, 0.0, 0.0, 0.0")
                DEFAULT_VALUE(mx::Matrix33, "1.0,0.0,0.0, 0.0,1.0,0.0, 0.0,0.0,1.0")
                DEFAULT_VALUE(mx::Matrix44, "1.0,0.0,0.0,0.0, 0.0,1.0,0.0,0.0, 0.0,0.0,1.0,0.0, 0.0,0.0,0.0,1.0")
#undef DEFAULT_VALUE

                return create_variable(
                    create_value<StringValue>(std::move(value), std::move(type))
                );
            }
        }
    }

    VarPtr DefaultExpression::evaluate_impl() const
    {
        return get_default_value(type_);
    }

    string DefaultExpression::to_string() const
    {
        if (type_)
            return "default(" + type_->to_string() + ")";
        else
            return "default";
    }
}

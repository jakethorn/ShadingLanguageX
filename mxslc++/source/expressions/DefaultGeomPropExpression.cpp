//
// Created by jaket on 15/09/2026.
//

#include "expressions/DefaultGeomPropExpression.h"

#include "expressions/interface.h"
#include "runtime/interface.h"
#include "runtime/Runtime.h"
#include "runtime/Type.h"
#include "serialize/values/DefaultGeomPropValue.h"
#include "serialize/values/interface.h"

namespace mxslc::expressions
{
    DefaultGeomPropExpression::DefaultGeomPropExpression(string name, Token token) : Expression{std::move(token)}, name_{std::move(name)}
    {

    }

    ExprPtr DefaultGeomPropExpression::monomorphize(const TypePtr& template_type) const
    {
        return create_expression<DefaultGeomPropExpression>(name_, token_);
    }

    void DefaultGeomPropExpression::init_impl(const vector<TypePtr>& types)
    {
        const mx::DocumentPtr doc = serializer().document();
        geomprop_ = doc->getGeomPropDef(name_);

        if (geomprop_ == nullptr)
            throw CompileError{"Unknown defaultgeomprop name: '" + name_ + "'"};
    }

    TypePtr DefaultGeomPropExpression::type_impl() const
    {
        return Type::of(geomprop_);
    }

    VarPtr DefaultGeomPropExpression::evaluate_impl() const
    {
        return create_variable(
            create_value<DefaultGeomPropValue>(geomprop_)
        );
    }

    string DefaultGeomPropExpression::to_string() const
    {
        return "geomprop(" + name_ + ")";
    }
}

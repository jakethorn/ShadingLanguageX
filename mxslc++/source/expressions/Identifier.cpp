//
// Created by jaket on 28/11/2025.
//

#include "expressions/Identifier.h"

#include "expressions/interface.h"
#include "runtime/FunctionQuery.h"
#include "runtime/Scope.h"
#include "runtime/utils/FunctionResolver.h"
#include "runtime/variables/Variable.h"
#include "runtime/utils/invoke.h"

namespace mxslc::expressions
{
    ExprPtr Identifier::monomorphize(const TypePtr& template_type) const
    {
        return create_expression<Identifier>(token_);
    }

    void Identifier::init_impl(const vector<TypePtr>& types)
    {
        if (scope().has_variable(name_))
            var_ = scope().get_variable(name_);

        if (scope().has_function({types, name_, /*is_parameterless*/true}))
        {
            if (var_)
                throw CompileError{"Identifier '" + name_ + "' is defined as both a variable and a parameterless function"};

            var_ = runtime_utils::invoke_function(types, name_);
        }

        if (var_ == nullptr)
        {
            if (scope().has_function({name_, /*is_parameterless*/true}))
                runtime_utils::resolve_function({types, name_, /*is_parameterless*/true});
            else
                throw CompileError{"Variable or parameterless function not defined: " + name_};
        }
    }

    TypePtr Identifier::type_impl() const
    {
        return var_->type();
    }

    VarPtr Identifier::evaluate_impl() const
    {
        return var_;
    }

    string Identifier::to_string() const
    {
        return name_;
    }
}

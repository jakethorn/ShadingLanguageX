//
// Created by jaket on 05/05/2026.
//

#include "expressions/NamedConstructor.h"

#include "expressions/FunctionCall.h"
#include "expressions/interface.h"
#include "runtime/Scope.h"
#include "runtime/Type.h"
#include "runtime/utils/monomorphize.h"

namespace mxslc::expressions
{
    NamedConstructor::NamedConstructor(string name, ArgumentList args)
        : name_{std::move(name)}, args_{std::move(args)}
    {

    }

    NamedConstructor::NamedConstructor(string name, ArgumentList args, Token token)
        : Expression{std::move(token)}, name_{std::move(name)}, args_{std::move(args)}
    {

    }

    ExprPtr NamedConstructor::monomorphize(const TypePtr& template_type) const
    {
        auto&& [name, args] = runtime_utils::monomorphize_all(template_type, name_, args_);
        return create_expression<NamedConstructor>(std::move(name), std::move(args), token_);
    }

    void NamedConstructor::init_impl(const vector<TypePtr>& types)
    {
        const TypePtr type = scope().get_type(name_);
        func_call_ = create_expression<FunctionCall>("__" + type->name() + "__", std::move(args_), token_);
        func_call_->init(type);
    }

    TypePtr NamedConstructor::type_impl() const
    {
        return func_call_->type();
    }

    VarPtr NamedConstructor::evaluate_impl() const
    {
        return func_call_->evaluate();
    }
}

//
// Created by jaket on 30/03/2026.
//

#ifndef MXSLC_FORLOOP_H
#define MXSLC_FORLOOP_H

#include "Statement.h"
#include "Token.h"
#include "expressions/Expression.h"
#include "runtime/Runtime.h"
#include "runtime/ModifierList.h"
#include "runtime/Scope.h"
#include "runtime/Variable2.h"

class ForRangeLoop final : public Statement
{
public:
    ForRangeLoop(Token token, ModifierList mods, TypeInfoPtr type, string name, ExprPtr lower_expr, ExprPtr step_expr, ExprPtr upper_expr, StmtPtr body);

    StmtPtr instantiate_template_types(const TypeInfoPtr& template_type) const override;

protected:
    void execute_impl() const override;

private:
    template<typename T>
    void execute_with() const
    {
        const TypeInfoPtr type = runtime().scope().resolve_type(type_);

        lower_expr_->init(type);
        if (step_expr_)
            step_expr_->init(type);
        upper_expr_->init(type);

        T lower = lower_expr_->evaluate()->value_as<T>();
        T step = step_expr_ ? step_expr_->evaluate()->value_as<T>() : 1;
        T upper = upper_expr_->evaluate()->value_as<T>();

        while (lower <= upper)
        {
            runtime().enter_inline_scope();
            VarPtr2 var = std::make_shared<Variable2>(mods_, type, std::make_shared<BasicValue>(lower));
            runtime().scope().add_variable(name_, std::move(var));
            body_->execute();
            runtime().exit_scope();
            lower += step;
        }
    }

    ModifierList mods_;
    TypeInfoPtr type_;
    string name_;
    ExprPtr lower_expr_;
    ExprPtr step_expr_;
    ExprPtr upper_expr_;
    StmtPtr body_;
};

#endif //MXSLC_FORLOOP_H

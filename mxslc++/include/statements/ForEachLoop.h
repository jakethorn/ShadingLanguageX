//
// Created by jaket on 30/03/2026.
//

#ifndef MXSLC_FOREACHLOOP_H
#define MXSLC_FOREACHLOOP_H

#include "statements/Statement.h"
#include "runtime/ModifierList.h"

namespace mxslc::statements
{
    class ForEachLoop final : public Statement
    {
    public:
        ForEachLoop(Token token, ModifierList mods, TypePtr type, string name, ExprPtr iter_expr, StmtPtr body);

        const string& name() const { return name_; }
        // the type as it is written, i.e., before it is resolved
        const TypePtr& declared_type() const { return type_; }
        const ExprPtr& iter_expression() const { return iter_expr_; }
        StmtPtr with_body(ExprPtr iter_expr, StmtPtr body) const;

        StmtPtr monomorphize(const TypePtr& template_type) const override;

        string to_string() const override;
        bool is_block() const override { return true; }

        bool is_hinted() const override { return true; }
        string hint_skeleton() const override;

    protected:
        void init() override;
        void execute_impl() const override;

    private:
        ModifierList mods_;
        TypePtr type_;
        string name_;
        ExprPtr iter_expr_;
        StmtPtr body_;
        string type_string_;
        mutable vector<string> hint_values_;
    };
}

#endif //MXSLC_FOREACHLOOP_H

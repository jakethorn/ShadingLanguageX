//
// Created by jaket on 16/01/2026.
//

#ifndef MXSLC_USINGDECLARATION_H
#define MXSLC_USINGDECLARATION_H

#include "statements/Statement.h"

namespace mxslc::statements
{
    class UsingDeclaration final : public Statement
    {
    public:
        UsingDeclaration(Token token, string name, TypePtr type);

        const string& name() const { return name_; }
        const TypePtr& type() const { return type_; }

        StmtPtr monomorphize(const TypePtr& template_type) const override;

        string to_string() const override;

        bool is_hinted() const override { return true; }
        string hint_skeleton() const override { return "using " + name_ + " = " + type_string_ + ";"; }

    protected:
        void execute_impl() const override;

    private:
        string name_;
        TypePtr type_;
        // the type as it is written, before it is resolved
        string type_string_;
    };
}

#endif //MXSLC_USINGDECLARATION_H

//
// Created by jaket on 09/11/2026.
//

#ifndef MXSLC_NAMESPACEDEFINITION_H
#define MXSLC_NAMESPACEDEFINITION_H

#include "statements/Statement.h"

namespace mxslc::statements
{
    class NamespaceDefinition final : public Statement
    {
    public:
        NamespaceDefinition(string name, StmtPtr body, Token token = {});

        StmtPtr monomorphize(const TypePtr& template_type) const override;

        string to_string() const override;

    protected:
        void execute_impl() const override;

    private:
        string name_;
        StmtPtr body_;
    };
}

#endif //MXSLC_NAMESPACEDEFINITION_H

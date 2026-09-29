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
        UsingDeclaration(Token token, string name, TypePtr type)
            : Statement{std::move(token)}, name_{std::move(name)}, type_{std::move(type)} { }

        StmtPtr monomorphize(const TypePtr& template_type) const override;

        string to_string() const override;

    protected:
        void execute_impl() const override;

    private:
        string name_;
        TypePtr type_;
    };
}

#endif //MXSLC_USINGDECLARATION_H

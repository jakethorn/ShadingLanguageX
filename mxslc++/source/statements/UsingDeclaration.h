//
// Created by jaket on 16/01/2026.
//

#ifndef MXSLC_USINGDECLARATION_H
#define MXSLC_USINGDECLARATION_H

#include "Statement.h"
#include "Token.h"

class UsingDeclaration final : public Statement
{
public:
    UsingDeclaration(Token token, string name, TypeInfoPtr type)
        : Statement{std::move(token)}, name_{std::move(name)}, type_{std::move(type)} { }

    StmtPtr instantiate_template_types(const TypeInfoPtr& template_type) const override;

protected:
    void execute_impl() const override;

private:
    string name_;
    TypeInfoPtr type_;
};

#endif //MXSLC_USINGDECLARATION_H

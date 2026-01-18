//
// Created by jaket on 16/01/2026.
//

#ifndef MXSLC_USINGDECLARATION_H
#define MXSLC_USINGDECLARATION_H

#include "Statement.h"
#include "Token.h"
#include "runtime/Type.h"

class UsingDeclaration final : public Statement
{
public:
    UsingDeclaration(const Runtime& runtime, Token name, TypeInfoPtr type) : Statement{runtime}, name_{std::move(name)}, type_{std::move(type)} { }

    [[nodiscard]] StmtPtr instantiate_template_types(const TypeInfoPtr& template_type) const override;
    void execute() const override;

private:
    Token name_;
    TypeInfoPtr type_;
};

#endif //MXSLC_USINGDECLARATION_H

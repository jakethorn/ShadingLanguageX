//
// Created by jaket on 11/01/2026.
//

#ifndef MXSLC_NOSTATEMENT_H
#define MXSLC_NOSTATEMENT_H

#include "Statement.h"

class NoStatement final : public Statement
{
public:
    explicit NoStatement(const Runtime& runtime) : Statement(runtime) { }

    [[nodiscard]] StmtPtr instantiate_template_types(const Type& template_type) const override { return std::make_unique<NoStatement>(runtime_); }
    void execute() const override { }
};

#endif //MXSLC_NOSTATEMENT_H
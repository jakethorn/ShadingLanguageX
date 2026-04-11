//
// Created by jaket on 10/04/2026.
//

#ifndef MXSLC_BLOCKSTATEMENT_H
#define MXSLC_BLOCKSTATEMENT_H

#include "Statement.h"

class BlockStatement final : public Statement
{
public:
    BlockStatement(const Runtime& runtime, Token token, vector<StmtPtr> body);

    StmtPtr instantiate_template_types(const TypeInfoPtr& template_type) const override;

protected:
    void execute_impl() const override;

private:
    vector<StmtPtr> body_;
};

#endif //MXSLC_BLOCKSTATEMENT_H

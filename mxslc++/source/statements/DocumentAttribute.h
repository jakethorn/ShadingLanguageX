//
// Created by jaket on 05/05/2026.
//

#ifndef MXSLC_DOCUMENTATTRIBUTE_H
#define MXSLC_DOCUMENTATTRIBUTE_H

#include "Statement.h"

class DocumentAttribute final : public Statement
{
public:
    DocumentAttribute(Token token, Attribute attr);

    StmtPtr instantiate_template_types(const TypePtr& template_type) const override;

protected:
    void execute_impl() const override;

private:
    Attribute attr_;
};

#endif //MXSLC_DOCUMENTATTRIBUTE_H
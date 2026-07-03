//
// Created by jaket on 05/05/2026.
//

#ifndef MXSLC_DOCUMENTATTRIBUTE_H
#define MXSLC_DOCUMENTATTRIBUTE_H

#include "statements/Statement.h"

namespace mxslc::statements
{
    class DocumentAttribute final : public Statement
    {
    public:
        DocumentAttribute(Token token, Attribute attr);

        StmtPtr monomorphize(const TypePtr& template_type) const override;

        string to_string() const override;

    protected:
        void execute_impl() const override;

    private:
        Attribute attr_;
    };
}

#endif //MXSLC_DOCUMENTATTRIBUTE_H
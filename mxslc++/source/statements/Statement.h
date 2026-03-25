//
// Created by jaket on 06/11/2025.
//

#ifndef FENNEC_STATEMENT_H
#define FENNEC_STATEMENT_H

#include "utils/common.h"

class Runtime;

class Statement
{
public:
    explicit Statement(const Runtime& runtime) : runtime_{runtime} { }
    virtual ~Statement() = default;

    [[nodiscard]] virtual StmtPtr instantiate_template_types(const TypeInfoPtr& template_type) const = 0;
    virtual void execute() const = 0;

protected:
    const Runtime& runtime_;
};

#endif //FENNEC_STATEMENT_H

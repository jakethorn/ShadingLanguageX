//
// Created by jaket on 05/05/2026.
//

#include "statements/DocumentAttribute.h"

#include "mtlx/MtlXSerializer.h"
#include "statements/interface.h"

namespace mxslc::statements
{
    DocumentAttribute::DocumentAttribute(Token token, Attribute attr) : Statement{std::move(token)}, attr_{std::move(attr)}
    {

    }

    StmtPtr DocumentAttribute::monomorphize(const TypePtr& template_type) const
    {
        return create_statement<DocumentAttribute>(token_, attr_);
    }

    void DocumentAttribute::execute_impl() const
    {
        attr_.add_to(serializer().document());
    }
}

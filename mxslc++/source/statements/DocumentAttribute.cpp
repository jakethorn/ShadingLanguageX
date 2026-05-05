//
// Created by jaket on 05/05/2026.
//

#include "DocumentAttribute.h"

#include "runtime/Runtime.h"

DocumentAttribute::DocumentAttribute(Token token, Attribute attr) : Statement{std::move(token)}, attr_{std::move(attr)}
{

}

StmtPtr DocumentAttribute::instantiate_template_types(const TypePtr& template_type) const
{
    return std::make_unique<DocumentAttribute>(token_, attr_);
}

void DocumentAttribute::execute_impl() const
{
    attr_.add_to(Runtime::get().serializer().document());
}

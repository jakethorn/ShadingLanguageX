//
// Created by jaket on 16/01/2026.
//

#include "UsingDeclaration.h"
#include "runtime/TypeInfo.h"

StmtPtr UsingDeclaration::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    TypeInfoPtr type = type_->instantiate_template_types(template_type);
    return std::make_unique<UsingDeclaration>(runtime_, name_, type);
}

void UsingDeclaration::execute() const
{

}

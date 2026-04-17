//
// Created by jaket on 16/01/2026.
//

#include "UsingDeclaration.h"

#include "runtime/Runtime.h"
#include "runtime/Scope.h"
#include "runtime/TypeInfo.h"

StmtPtr UsingDeclaration::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    TypeInfoPtr type = type_->instantiate_template_types(template_type);
    return std::make_unique<UsingDeclaration>(token_, name_, type);
}

void UsingDeclaration::execute_impl() const
{
    runtime().scope().add_alias(name_, type_);
}

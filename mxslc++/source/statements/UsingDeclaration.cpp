//
// Created by jaket on 16/01/2026.
//

#include "statements/UsingDeclaration.h"

#include "runtime/Runtime.h"
#include "runtime/Scope.h"
#include "runtime/Type.h"
#include "statements/interface.h"

namespace mxslc::statements
{
    StmtPtr UsingDeclaration::monomorphize(const TypePtr& template_type) const
    {
        TypePtr type = type_->monomorphize(template_type);
        return create_statement<UsingDeclaration>(token_, name_, std::move(type));
    }

    void UsingDeclaration::execute_impl() const
    {
        scope().add_alias(name_, type_);
    }
}

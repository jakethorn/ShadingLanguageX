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
        return create_statement<UsingDeclaration>(name_, std::move(type), token_);
    }

    void UsingDeclaration::execute_impl() const
    {
        scope().add_alias(name_, type_);
    }

    void UsingDeclaration::recover() const
    {
        if (not scope().has_type(name_))
            scope().poison(name_);
    }

    string UsingDeclaration::to_string() const
    {
        return "using " + name_ + " = " + type_->to_string();
    }
}

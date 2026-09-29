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
    UsingDeclaration::UsingDeclaration(Token token, string name, TypePtr type)
        : Statement{std::move(token)}, name_{std::move(name)}, type_{std::move(type)}, type_string_{type_ ? type_->to_string() : ""}
    {

    }

    StmtPtr UsingDeclaration::monomorphize(const TypePtr& template_type) const
    {
        TypePtr type = type_->monomorphize(template_type);
        return create_statement<UsingDeclaration>(token_, name_, std::move(type));
    }

    void UsingDeclaration::execute_impl() const
    {
        scope().add_alias(name_, type_);
    }

    string UsingDeclaration::to_string() const
    {
        return "using " + name_ + " = " + type_->to_string() + ";";
    }
}

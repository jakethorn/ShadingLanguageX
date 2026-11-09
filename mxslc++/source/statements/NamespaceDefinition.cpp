//
// Created by jaket on 09/11/2026.
//

#include "statements/NamespaceDefinition.h"

#include "runtime/Scope.h"
#include "statements/interface.h"

namespace mxslc::statements
{
    NamespaceDefinition::NamespaceDefinition(string name, StmtPtr body, Token token)
        : Statement{std::move(token)}, name_{std::move(name)}, body_{std::move(body)} { }

    StmtPtr NamespaceDefinition::monomorphize(const TypePtr& template_type) const
    {
        return create_statement<NamespaceDefinition>(name_, body_->monomorphize(template_type), token_);
    }

    void NamespaceDefinition::execute_impl() const
    {
        scope().enter_namespace(name_);
        body_->execute();
        scope().exit_namespace();
    }

    string NamespaceDefinition::to_string() const
    {
        return "namespace" + name_ + "\n" + body_->to_string();
    }
}

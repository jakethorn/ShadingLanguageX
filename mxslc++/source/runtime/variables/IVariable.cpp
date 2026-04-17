//
// Created by jaket on 15/04/2026.
//

#include "IVariable.h"

#include <cassert>

#include "ChildVariable.h"
#include "runtime/Runtime.h"
#include "values/Value.h"

void IVariable::validate(const ValuePtr& value) const
{
    assert(value->parent_graph() == nullptr or value->parent_graph() == runtime_.serializer().graph());
}

bool IVariable::is_inline() const
{
    return runtime_.scope().is_variable_inline(self());
}

VarPtr IVariable::child(size_t index)
{
    return std::make_shared<ChildVariable>(runtime_, self(), index);
}

VarPtr IVariable::child(const string& name)
{
    return std::make_shared<ChildVariable>(runtime_, self(), name);
}

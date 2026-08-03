//
// Created by jaket on 13/07/2026.
//

#include "runtime/variables/ComponentVariable.h"
#include "runtime/ArgumentList.h"
#include "expressions/interface.h"
#include "expressions/FunctionCall.h"
#include "expressions/RuntimeExpression.h"
#include "runtime/interface.h"
#include "runtime/utils/invoke.h"

namespace mxslc::runtime
{
    ComponentVariable::ComponentVariable(VarPtr value, VarPtr index)
        : Variable{TokenType::Mutable, get_type(value, index)}, value_{std::move(value)}, index_{std::move(index)}
    {

    }

    ValuePtr ComponentVariable::value_impl() const
    {
        if (component_value_ == nullptr)
            component_value_ = runtime_utils::invoke_function("__get__", ArgumentList{value_, index_})->value();
        return component_value_;
    }

    void ComponentVariable::copy_value_impl(ValuePtr value)
    {
        runtime_utils::invoke_function("__set__", ArgumentList{value_, index_, as_expression(std::move(value))});
    }

    void ComponentVariable::set_node_name(const string& name) const
    {

    }

    TypePtr ComponentVariable::get_type(const VarPtr& value, const VarPtr& index)
    {
        const FunctionCallPtr getter = create_expression<FunctionCall>("__get__", ArgumentList{value, index});
        getter->init();
        return getter->type();
    }
}

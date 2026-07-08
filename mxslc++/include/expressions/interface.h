//
// Created by jaket on 07/07/2026.
//

#ifndef MXSLC_EXPRESSIONS_INTERFACE_H
#define MXSLC_EXPRESSIONS_INTERFACE_H

#include "common.h"

namespace mxslc::expressions
{
    template<typename T, typename... Args>
    shared_ptr<T> create_expression(Args&&... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    template<typename T>
    shared_ptr<T> cast_expression(const ExprPtr& expr)
    {
        return std::dynamic_pointer_cast<T>(expr);
    }

#define TYPE_DEF(T) class T; \
using T##Ptr = shared_ptr<T>;

    TYPE_DEF(CompoundAssignment)
    TYPE_DEF(DotOperator)
    TYPE_DEF(FunctionCall)
    TYPE_DEF(Identifier)
    TYPE_DEF(IncrementOperator)
    TYPE_DEF(IndexingOperator)
    TYPE_DEF(Literal)
    TYPE_DEF(MethodCall)
    TYPE_DEF(NamedConstructor)
    TYPE_DEF(NullExpression)
    TYPE_DEF(RangeExpression)
    TYPE_DEF(RuntimeExpression)
    TYPE_DEF(ThisExpression)
    TYPE_DEF(TypeOfOperator)
    TYPE_DEF(UnnamedConstructor)
    TYPE_DEF(VariableDefinitionExpression)

#undef TYPE_DEF
}

#endif //MXSLC_EXPRESSIONS_INTERFACE_H

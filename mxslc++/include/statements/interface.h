//
// Created by jaket on 07/07/2026.
//

#ifndef MXSLC_STATEMENTS_INTERFACE_H
#define MXSLC_STATEMENTS_INTERFACE_H

#include "common.h"

namespace mxslc::statements
{
    template<typename T, typename... Args>
    unique_ptr<T> create_statement(Args&&... args)
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

#define TYPE_DEF(T) class T; \
using T##Ptr = unique_ptr<T>;

    TYPE_DEF(BlockStatement)
    TYPE_DEF(ClassDefinition)
    TYPE_DEF(ConstructorDefinition)
    TYPE_DEF(DocumentAttribute)
    TYPE_DEF(ExpressionStatement)
    TYPE_DEF(ForEachLoop)
    TYPE_DEF(FunctionDefinition)
    TYPE_DEF(IfStatement)
    TYPE_DEF(MultiVariableDefinition)
    TYPE_DEF(PrintStatement)
    TYPE_DEF(UsingDeclaration)
    TYPE_DEF(VariableAssignment)
    TYPE_DEF(VariableDefinition)

#undef TYPE_DEF
}

#endif //MXSLC_STATEMENTS_INTERFACE_H

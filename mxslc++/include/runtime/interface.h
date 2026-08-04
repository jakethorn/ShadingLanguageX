//
// Created by jaket on 07/07/2026.
//

#ifndef MXSLC_RUNTIME_INTERFACE_H
#define MXSLC_RUNTIME_INTERFACE_H

#include "common.h"
#include "Primitive.h"
#include "runtime/ModifierList.h"

namespace mxslc::runtime
{
    class Scope;
    using ScopePtr = unique_ptr<Scope>;

    class Variable;
    using VarPtr = shared_ptr<Variable>;

    class Function;
    using FuncPtr = shared_ptr<Function>;

    class Type;
    using TypePtr = shared_ptr<Type>;

    class Parameter;
    using ParameterValues = vector<std::pair<const Parameter&, VarPtr>>;

    template<typename... Args>
    FuncPtr create_function(Args&&... args)
    {
        return std::make_shared<Function>(std::forward<Args>(args)...);
    }

    template<typename... Args>
    TypePtr create_type(Args&&... args)
    {
        return std::make_shared<Type>(std::forward<Args>(args)...);
    }

    template<typename... Args>
    ScopePtr create_scope(Args&&... args)
    {
        return std::make_unique<Scope>(std::forward<Args>(args)...);
    }

    VarPtr create_variable(ModifierList mods, TypePtr type, const vector<VarPtr>& children);
    VarPtr create_variable(ModifierList mods, TypePtr type, const vector<Primitive>& children);
    VarPtr create_variable(ModifierList mods, TypePtr type, ValuePtr value);
    VarPtr create_variable(ModifierList mods, TypePtr type, const VarPtr& value);
    VarPtr create_variable(ModifierList mods, ValuePtr value);
    VarPtr create_variable(ModifierList mods, Primitive value);

    VarPtr create_variable(TypePtr type, const vector<VarPtr>& children);
    VarPtr create_variable(TypePtr type, ValuePtr value);
    VarPtr create_variable(TypePtr type, const VarPtr& value);

    VarPtr create_variable(const vector<VarPtr>& children);
    VarPtr create_variable(const vector<Primitive>& children);
    VarPtr create_variable(ValuePtr value);
    VarPtr create_variable(Primitive value);
    VarPtr create_variable(const VarPtr& value);

    VarPtr create_variable(ModifierList mods, TypePtr type);
    VarPtr create_variable(TypePtr type);
}

#endif //MXSLC_RUNTIME_INTERFACE_H

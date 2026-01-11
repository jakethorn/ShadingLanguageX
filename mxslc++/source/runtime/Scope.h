//
// Created by jaket on 28/11/2025.
//

#ifndef FENNEC_SCOPE_H
#define FENNEC_SCOPE_H

#include "utils/common.h"
#include "Variable.h"
#include "Function.h"

class Token;
class ArgumentList;

class Scope
{
public:
    Scope();
    explicit Scope(ScopePtr parent);

    ScopePtr parent()
    {
        return std::move(parent_);
    }

    void add_variable(Variable&& var);
    void set_variable(Variable&& var);
    [[nodiscard]] const Variable& get_variable(const Token& name) const;

    void add_function(FuncPtr func);
    [[nodiscard]] vector<const Function*> get_functions(
        const vector<Type>& return_types,
        const Token& name,
        const optional<Type>& template_type,
        const ArgumentList& args
    ) const;
    [[nodiscard]] const Function& get_function(
        const vector<Type>& return_types,
        const Token& name,
        const optional<Type>& template_type,
        const ArgumentList& args
    ) const;

    void add_type(Type&& type);
    [[nodiscard]] bool has_type(const Type& type) const;
    [[nodiscard]] bool has_type(const string& name) const;

private:
    ScopePtr parent_;

    unordered_map<string, Variable> variables_;
    vector<FuncPtr> functions_;
    unordered_map<string, Type> types_;
};

#endif //FENNEC_SCOPE_H

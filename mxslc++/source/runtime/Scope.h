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
    [[nodiscard]] vector<FuncPtr> get_functions(
        const vector<TypeInfoPtr>& return_types,
        const Token& name,
        const TypeInfoPtr& template_type,
        const ArgumentList& args
    ) const;
    [[nodiscard]] FuncPtr get_function(
        const vector<TypeInfoPtr>& return_types,
        const Token& name,
        const TypeInfoPtr& template_type,
        const ArgumentList& args
    ) const;

    void add_type(TypeInfoPtr type);
    void add_basic_type(const string& name);
    void add_alias(const Token& name, TypeInfoPtr type);
    [[nodiscard]] bool has_type(const Token& name) const;
    [[nodiscard]] bool has_type(const string& name) const;
    [[nodiscard]] TypeInfoPtr resolve_type(const TypeInfoPtr& type) const;
    void resolve_fields(const TypeInfoPtr& type) const;
    [[nodiscard]] TypeInfoPtr get_type(const Token& name) const;
    [[nodiscard]] TypeInfoPtr get_type(const string& name) const;
    [[nodiscard]] TypeInfoPtr get_type(const basic_t& val) const;

private:
    ScopePtr parent_;

    unordered_map<string, Variable> variables_;
    vector<FuncPtr> functions_;
    unordered_map<string, TypeInfoPtr> types_;
};

#endif //FENNEC_SCOPE_H

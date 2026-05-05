//
// Created by jaket on 07/11/2025.
//

#ifndef FENNEC_ARGUMENT_H
#define FENNEC_ARGUMENT_H

#include "AttributeList.h"
#include "ModifierList.h"
#include "utils/common.h"

class Parameter;

class Argument
{
public:
    Argument(AttributeList attrs, ModifierList mods, string name, ExprPtr expr, size_t index);
    Argument(ModifierList mods, string name, ExprPtr expr, size_t index);
    Argument(ModifierList mods, ExprPtr expr, size_t index);
    Argument(string name, ExprPtr expr, size_t index);
    Argument(ExprPtr expr, size_t index);

    Argument(Argument&& other) noexcept;

    ~Argument();

    const AttributeList& attributes() const { return attrs_; }
    const ModifierList& modifiers() const { return mods_; }
    bool has_name() const { return not name_.empty(); }
    const string& name() const { return name_; }
    ExprPtr expression() const { return expr_; }
    size_t index() const { return index_; }

    Argument instantiate_template_types(const TypePtr& template_type) const;
    void init(const TypePtr& type) const;
    void init(const vector<TypePtr>& types) const;
    bool try_init(const vector<TypePtr>& types) const;
    bool is_initialized() const;
    TypePtr type() const;
    VarPtr evaluate() const;

    void validate(const Parameter& param) const;

private:
    AttributeList attrs_;
    ModifierList mods_;
    string name_;
    ExprPtr expr_;
    size_t index_;
};

#endif //FENNEC_ARGUMENT_H

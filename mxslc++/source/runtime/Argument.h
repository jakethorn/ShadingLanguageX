//
// Created by jaket on 07/11/2025.
//

#ifndef FENNEC_ARGUMENT_H
#define FENNEC_ARGUMENT_H

#include "utils/common.h"

class Parameter;

class Argument
{
public:
    Argument(ExprPtr expr, size_t index);
    Argument(string name, ExprPtr expr, size_t index);
    Argument(bool is_out, string name, ExprPtr expr, size_t index);

    Argument(Argument&& other) noexcept;

    ~Argument();

    bool is_out() const { return is_out_; }
    bool has_name() const { return not name_.empty(); }
    const string& name() const { return name_; }
    ExprPtr expression() const { return expr_; }
    size_t index() const { return index_; }

    Argument instantiate_template_types(const TypeInfoPtr& template_type) const;
    void init(const TypeInfoPtr& type) const;
    void init(const vector<TypeInfoPtr>& types) const;
    bool try_init(const vector<TypeInfoPtr>& types) const;
    bool is_initialized() const;
    TypeInfoPtr type() const;
    ValuePtr evaluate() const;

    void validate(const Parameter& param) const;

private:
    bool is_out_;
    string name_;
    ExprPtr expr_;
    size_t index_;
};

#endif //FENNEC_ARGUMENT_H

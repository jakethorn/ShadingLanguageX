//
// Created by jaket on 07/11/2025.
//

#ifndef FENNEC_ARGUMENT_H
#define FENNEC_ARGUMENT_H

#include "utils/common.h"

#include "Token.h"

class Argument
{
public:
    Argument(ExprPtr expr, size_t index);
    Argument(optional<string> name, ExprPtr expr, size_t index);

    Argument(Argument&& other) noexcept;

    ~Argument();

    [[nodiscard]] bool has_name() const { return name_.has_value(); }
    [[nodiscard]] const string& name() const { return name_.value(); }
    [[nodiscard]] size_t index() const { return index_; }

    [[nodiscard]] Argument instantiate_template_types(const TypeInfoPtr& template_type) const;
    void init(const vector<TypeInfoPtr>& types) const;
    [[nodiscard]] bool try_init(const vector<TypeInfoPtr>& types) const;
    [[nodiscard]] bool is_initialized() const;
    [[nodiscard]] TypeInfoPtr type() const;
    [[nodiscard]] ValuePtr evaluate() const;

private:
    optional<string> name_;
    ExprPtr expr_;
    size_t index_;
};

#endif //FENNEC_ARGUMENT_H

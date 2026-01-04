//
// Created by jaket on 31/12/2025.
//

#ifndef FENNEC_NULLVALUE_H
#define FENNEC_NULLVALUE_H

#include "Value.h"
#include "runtime/Type.h"

class NullValue final : public Value
{
public:
    NullValue(Type type) : type_{std::move(type)} { }

    [[nodiscard]] const Type& type() const override { return type_; }
    [[nodiscard]] string str() const override { return "null"s; }

private:
    Type type_;
};

#endif //FENNEC_NULLVALUE_H

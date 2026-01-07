//
// Created by jaket on 07/01/2026.
//

#ifndef MXSLC_INTERFACEVALUE_H
#define MXSLC_INTERFACEVALUE_H

#include "utils/common.h"
#include "Value.h"
#include "runtime/Type.h"

class InterfaceValue final : public Value
{
public:
    explicit InterfaceValue(string name, Type type) : name_{std::move(name)}, type_{std::move(type)} { }

    [[nodiscard]] const Type& type() const override { return type_; }
    [[nodiscard]] string str() const override { return name_; }

private:
    string name_;
    Type type_;
};

#endif //MXSLC_INTERFACEVALUE_H

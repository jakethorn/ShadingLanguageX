//
// Created by jaket on 12/11/2025.
//

#ifndef FENNEC_VALUE_H
#define FENNEC_VALUE_H

class Type;

class Value
{
public:
    Value() = default;

    virtual ~Value() = default;

    [[nodiscard]] virtual const Type& type() const = 0;
    [[nodiscard]] virtual string str() const = 0;
};

#endif //FENNEC_VALUE_H

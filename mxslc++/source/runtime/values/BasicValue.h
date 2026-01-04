//
// Created by jaket on 27/11/2025.
//

#ifndef FENNEC_BASICVALUE_H
#define FENNEC_BASICVALUE_H

#include "utils/common.h"
#include "Value.h"
#include "runtime/Type.h"

class BasicValue final : public Value
{
public:
    BasicValue(basic_t val) : val_{std::move(val)}, type_{val_} { }

    [[nodiscard]] const Type& type() const override { return type_; }
    [[nodiscard]] string str() const override
    {
        return std::visit(
            [](const auto& v){
                std::stringstream ss;
                ss << std::boolalpha << v;
                return ss.str();
            },
            val_
        );
    }

    template<typename T>
    [[nodiscard]] bool is() const
    {
        return std::holds_alternative<T>(val_);
    }

    template<typename T>
    [[nodiscard]] T get() const
    {
        return std::get<T>(val_);
    }

    [[nodiscard]] basic_t get() const { return val_; }

private:
    basic_t val_;
    Type type_;
};

#endif //FENNEC_BASICVALUE_H

//
// Created by jaket on 24/11/2025.
//

#ifndef FENNEC_ATTRIBUTE_H
#define FENNEC_ATTRIBUTE_H

#include "utils/common.h"
#include "Token.h"

class Attribute
{
public:
    Attribute(string name, string val) : Attribute{std::nullopt, std::move(name), std::move(val)} { }
    Attribute(optional<string> elem, string name, string val) : elem_{std::move(elem)}, name_{std::move(name)}, val_{std::move(val)} { }

    [[nodiscard]] bool has_element() const { return elem_.has_value(); }
    [[nodiscard]] const string& element() const { return elem_.value(); }
    [[nodiscard]] const string& name() const { return name_; }
    [[nodiscard]] const string& value() const { return val_; }

private:
    optional<string> elem_;
    string name_;
    string val_;
};

#endif //FENNEC_ATTRIBUTE_H

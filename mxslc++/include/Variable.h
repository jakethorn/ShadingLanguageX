//
// Created by jaket on 17/06/2026.
//

#ifndef MXSLC_INTERFACE_VARIABLE_H
#define MXSLC_INTERFACE_VARIABLE_H

#include <ciso646>
#include <memory>
#include <string>
#include <vector>

#include "primitive_t.h"

namespace mxslc::interface
{
    class Variable;
    using VariablePtr = std::shared_ptr<Variable>;

    class Variable
    {
    public:
        Variable(primitive_t value);
        Variable(const std::vector<primitive_t>& children);
        Variable(std::vector<VariablePtr> children);
        Variable(std::string name, primitive_t value);
        Variable(std::string name, const std::vector<primitive_t>& children);
        Variable(std::string name, std::vector<VariablePtr> children);
        Variable(std::string type, std::string name, primitive_t value);
        Variable(std::string type, std::string name, const std::vector<primitive_t>& children);
        Variable(std::string type, std::string name, std::vector<VariablePtr> children);

        bool has_type() const { return not type_.empty(); }
        const std::string& type() const { return type_; }

        bool has_name() const { return not name_.empty(); }
        const std::string& name() const { return name_; }

        bool has_value() const { return std::holds_alternative<primitive_t>(value_); }
        const primitive_t& value() const { return std::get<primitive_t>(value_); }

        bool has_children() const { return std::holds_alternative<std::vector<VariablePtr>>(value_); }
        const std::vector<VariablePtr>& children() const { return std::get<std::vector<VariablePtr>>(value_); }

    private:
        std::string type_;
        std::string name_;
        std::variant<primitive_t, std::vector<VariablePtr>> value_;
    };
}

#endif //MXSLC_INTERFACE_VARIABLE_H

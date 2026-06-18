//
// Created by jaket on 17/06/2026.
//

#include "Variable.h"

#include "runtime/Type.h"
#include "utils/common.h"

mxslc::Variable::Variable(primitive_t value) : Variable{TypeName::of(value), "", std::move(value)}
{

}

mxslc::Variable::Variable(const vector<primitive_t>& children) : Variable{"", "", children}
{

}

mxslc::Variable::Variable(vector<VariablePtr> children) : Variable{"", "", std::move(children)}
{

}

mxslc::Variable::Variable(string name, primitive_t value) : Variable{TypeName::of(value), std::move(name), std::move(value)}
{

}

mxslc::Variable::Variable(string name, const vector<primitive_t>& children) : Variable{"", std::move(name), children}
{

}

mxslc::Variable::Variable(string name, vector<VariablePtr> children) : Variable{"", std::move(name), std::move(children)}
{

}

mxslc::Variable::Variable(string type, string name, primitive_t value) : type_{std::move(type)}, name_{std::move(name)}, value_{std::move(value)}
{

}

mxslc::Variable::Variable(string type, string name, const vector<primitive_t>& children) : type_{std::move(type)}, name_{std::move(name)}
{
    vector<VariablePtr> value{};
    value.reserve(children.size());
    for (auto& child : children)
        value.emplace_back(std::make_shared<Variable>(child));
    value_ = std::move(value);
}

mxslc::Variable::Variable(string type, string name, vector<VariablePtr> children) : type_{std::move(type)}, name_{std::move(name)}, value_{std::move(children)}
{

}

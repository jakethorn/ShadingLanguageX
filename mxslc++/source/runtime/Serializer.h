//
// Created by jaket on 01/01/2026.
//

#ifndef FENNEC_EVALUATOR_H
#define FENNEC_EVALUATOR_H

#include "utils/common.h"

class Function;
class ArgumentList;

class Serializer
{
public:
    virtual ~Serializer() = default;

    [[nodiscard]] virtual ValuePtr write_function_call(const Function& func, const ArgumentList& args) const = 0;
    virtual void write_function(const Function& func) const = 0;
    virtual void save(const fs::path& filepath) const = 0;
};

#endif //FENNEC_EVALUATOR_H

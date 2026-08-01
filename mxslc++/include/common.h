//
// Created by jaket on 21/11/2025.
//

#ifndef FENNEC_COMMON_H
#define FENNEC_COMMON_H

#include <ciso646>
#include <algorithm>
#include <utility>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>

using std::unique_ptr;
using std::shared_ptr;
using std::weak_ptr;
using std::optional;
using std::string;
using std::string_view;
using std::vector;
using std::unordered_map;
using std::unordered_set;
namespace fs = std::filesystem;
using namespace std::string_literals;

#include <MaterialXCore/Generated.h>
namespace mx = MaterialX;

namespace mxslc
{
#define USING_NAMESPACE(N) namespace N { } \
    using namespace N;

    USING_NAMESPACE(preprocess)
    USING_NAMESPACE(statements)
    USING_NAMESPACE(expressions)
    USING_NAMESPACE(runtime)
    USING_NAMESPACE(serialize)
    USING_NAMESPACE(serialize::values)
    USING_NAMESPACE(decompile)

#undef USING_NAMESPACE

    namespace expressions
    {
        class Expression;
        using ExprPtr = shared_ptr<Expression>;
    }

    namespace statements
    {
        class Statement;
        using StmtPtr = unique_ptr<Statement>;
    }

    namespace runtime
    {
        class Scope;
        using ScopePtr = unique_ptr<Scope>;

        class Variable;
        using VarPtr = shared_ptr<Variable>;

        class Function;
        using FuncPtr = shared_ptr<Function>;

        class Type;
        using TypePtr = shared_ptr<Type>;

        class Parameter;
        using ParameterValues = vector<std::pair<const Parameter&, VarPtr>>;
    }

    namespace serialize::values
    {
        class Value;
        using ValuePtr = shared_ptr<Value>;
    }
}

#endif //FENNEC_COMMON_H

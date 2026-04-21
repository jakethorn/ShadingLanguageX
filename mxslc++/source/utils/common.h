//
// Created by jaket on 21/11/2025.
//

#ifndef FENNEC_COMMON_H
#define FENNEC_COMMON_H

// includes
#include <memory>
#include <optional>
#include <variant>
#include <string>
#include <vector>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>
#include <MaterialXCore/Node.h>

// namespaces
namespace fs = std::filesystem;
namespace mx = MaterialX;

using namespace std::string_literals;

namespace mxslc { }
using namespace mxslc;

using std::unique_ptr;
using std::shared_ptr;
using std::optional;
using std::string;
using std::string_view;
using std::vector;
using std::tuple;
using std::variant;
using std::unordered_map;
using std::unordered_set;

// forward declarations
class Statement;
using StmtPtr = unique_ptr<Statement>;

class Expression;
using ExprPtr = shared_ptr<Expression>;

class Value;
using ValuePtr = shared_ptr<Value>;

class Scope;
using ScopePtr = unique_ptr<Scope>;

//class IVariable;
//using VarPtr = shared_ptr<IVariable>;
//using ConstVarPtr = shared_ptr<const IVariable>;

class Variable2;
using VarPtr2 = shared_ptr<Variable2>;

//class Function;
//using FuncPtr = shared_ptr<Function>;

class Function2;
using FuncPtr2 = shared_ptr<Function2>;

class TypeInfo;
using TypeInfoPtr = shared_ptr<TypeInfo>;

// type aliases
using basic_t = variant<
    bool,
    int,
    float,
    string,
    mx::Vector2,
    mx::Vector3,
    mx::Vector4,
    mx::Color3,
    mx::Color4,
    mx::Matrix33,
    mx::Matrix44
>;

#endif //FENNEC_COMMON_H

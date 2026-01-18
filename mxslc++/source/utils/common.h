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
#include <MaterialXCore/Generated.h>

// namespaces
namespace fs = std::filesystem;
namespace mx = MaterialX;

using namespace std::string_literals;

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

class Function;
using FuncPtr = shared_ptr<Function>;

class TypeInfo;
using TypeInfoPtr = shared_ptr<TypeInfo>;

// type aliases
using basic_t = variant<bool, int, float, string>;

#endif //FENNEC_COMMON_H

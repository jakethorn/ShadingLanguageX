//
// Created by jaket on 21/11/2025.
//

#ifndef FENNEC_COMMON_H
#define FENNEC_COMMON_H

// includes
#include <ciso646>
#include <algorithm>
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

#include "CompileError.h"
#include "CompileOptions.h"
#include "parse_args.h"
#include "primitive_t.h"
#include "Span.h"
#include "TokenType.h"
#include "Token.h"
#include "TokenReader.h"

using mxslc::CompileError;
using mxslc::CompileOptions;
using mxslc::CommandLineArgs;
using mxslc::primitive_t;
using mxslc::Span;
using mxslc::TokenType;
using mxslc::Token;
using mxslc::TokenReader;

// namespaces
namespace fs = std::filesystem;
namespace mx = MaterialX;

using namespace std::string_literals;

using std::unique_ptr;
using std::shared_ptr;
using std::weak_ptr;
using std::optional;
using std::string;
using std::string_view;
using std::vector;
using std::tuple;
using std::pair;
using std::variant;
using std::unordered_map;
using std::unordered_set;

// forward declarations
class Scope;
using ScopePtr = unique_ptr<Scope>;

class Statement;
using StmtPtr = unique_ptr<Statement>;

class Expression;
using ExprPtr = shared_ptr<Expression>;

class Value;
using ValuePtr = shared_ptr<Value>;

class BasicValue;
using BasicValuePtr = shared_ptr<BasicValue>;

class Variable;
using VarPtr = shared_ptr<Variable>;

class Function;
using FuncPtr = shared_ptr<Function>;

class Type;
using TypePtr = shared_ptr<Type>;

class Parameter;
using ParameterValues = vector<pair<const Parameter&, VarPtr>>;

class Accessor;
using AccessorPtr = unique_ptr<Accessor>;

#endif //FENNEC_COMMON_H

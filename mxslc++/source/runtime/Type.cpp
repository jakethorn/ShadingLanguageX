//
// Created by jaket on 07/12/2025.
//

#include "Type.h"

#include "CompileError.h"
#include "Token.h"

Type::Type(const Token& token) : Type{token.lexeme()}
{

}

namespace
{
    Type to_type(const basic_t& value)
    {
        if (std::holds_alternative<bool>(value))
            return "bool"s;
        if (std::holds_alternative<int>(value))
            return "int"s;
        if (std::holds_alternative<float>(value))
            return "float"s;
        if (std::holds_alternative<string>(value))
            return "string"s;
        throw CompileError{"Invalid literal type"s};
    }
}

Type::Type(const basic_t& value) : Type{to_type(value)}
{

}

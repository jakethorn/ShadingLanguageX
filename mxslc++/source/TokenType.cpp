//
// Created by jaket on 03/07/2026.
//

#include "TokenType.h"
#include "common.h"
#include "utils/container_utils.h"

namespace mxslc
{
    using namespace container_utils;

    TokenType::Enum TokenType::to_enum(const char c)
    {
        const Enum e = static_cast<Enum>(c);
        if (contains(Symbols, e))
            return e;
        return Unknown;
    }

    TokenType::Enum TokenType::to_enum(const string& s)
    {
        if (s.size() == 1)
            return to_enum(s[0]);
        if (contains(Lexemes, s))
            return Lexemes.at(s);
        return Unknown;
    }

    bool TokenType::is_symbol() const
    {
        return contains(Symbols, enum_);
    }

    bool TokenType::is_compound_symbol() const
    {
        return contains(CompoundSymbols, enum_);
    }

    bool TokenType::is_keyword() const
    {
        return contains(Keywords, enum_);
    }

    string TokenType::to_string() const
    {
        for (const auto& [k, e] : Lexemes)
        {
            if (enum_ == e)
                return k;
        }

        return string{static_cast<char>(enum_)};
    }
}

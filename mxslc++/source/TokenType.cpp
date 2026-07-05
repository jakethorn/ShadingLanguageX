//
// Created by jaket on 03/07/2026.
//

#include "TokenType.h"
#include "utils/common.h"
#include "utils/template_utils.h"

namespace mxslc
{
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
        if (contains(lexemes_, s))
            return lexemes_.at(s);
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

    string TokenType::str() const
    {
        for (const auto& [k, e] : lexemes_)
        {
            if (enum_ == e)
            {
                return k;
            }
        }

        return string{static_cast<char>(enum_)};
    }
}

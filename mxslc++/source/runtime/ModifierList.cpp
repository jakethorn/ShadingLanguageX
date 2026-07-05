//
// Created by jaket on 11/04/2026.
//

#include "ModifierList.h"

#include "../../include/Token.h"
#include "CompileError.h"

ModifierList::ModifierList(const TokenType::Enum mod)
{
    add(mod);
}

ModifierList::ModifierList(const vector<Token>& mods)
{
    for (const Token token : mods)
    {
        const TokenType mod = token.type();
        if (not contains(mod))
            add(mod);
        else
            throw CompileError{"Multiple '" + mod.str() + "' modifiers"};
    }
}

string ModifierList::str() const
{
    string result;
    for (TokenType mod : mods_)
        result += mod.str() + " ";
    return result;
}

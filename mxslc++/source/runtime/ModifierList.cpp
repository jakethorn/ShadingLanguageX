//
// Created by jaket on 11/04/2026.
//

#include "ModifierList.h"

#include "Token.h"
#include "CompileError.h"

ModifierList::ModifierList(vector<TokenType> mods)
{
    for (const TokenType mod : mods)
    {
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

void ModifierList::throw_error(const string& msg)
{
    throw CompileError{msg};
}

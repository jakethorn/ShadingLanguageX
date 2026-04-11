//
// Created by jaket on 11/04/2026.
//

#include "ModifierList.h"

#include "CompileError.h"

ModifierList::ModifierList(vector<string> mods) : mods_{std::move(mods)}
{
    for (size_t i = 0; i < mods_.size(); ++i)
        for (size_t j = i+1; j < mods_.size(); ++j)
            if (mods_[i] == mods_[j])
                throw CompileError{"Multiple '" + mods_[j] + "' modifiers"};
}

string ModifierList::str() const
{
    string result;
    for (const string& mod : mods_)
        result += mod + " ";
    return result;
}

void ModifierList::throw_error(const string& msg)
{
    throw CompileError{msg};
}

//
// Created by jaket on 16/01/2026.
//

#ifndef MXSLC_MODIFIERLIST_H
#define MXSLC_MODIFIERLIST_H

#include "CompileError.h"
#include "utils/common.h"
#include "Token.h"

class ModifierList
{
public:
    ModifierList() = default;
    explicit ModifierList(vector<Token> mods) : mods_{std::move(mods)}
    {
        for (size_t i = 0; i < mods_.size(); ++i)
            for (size_t j = i+1; j < mods_.size(); ++j)
                if (mods_[i] == mods_[j])
                    throw CompileError{mods_[j], "Duplicate '" + mods_[j].lexeme() + "' modifiers"};
    }

    template<typename... Args>
    void validate(const Args&... valid_mods)
    {
        for (const Token& mod : mods_)
        {
            if (not (... || (mod == valid_mods)))
                throw CompileError{mod, "'" + mod.lexeme() + "' is not a valid modifier here"};
        }
    }

    [[nodiscard]] bool contains(const string& mod) const
    {
        return ::contains(mods_, mod);
    }

private:
    vector<Token> mods_;
};

#endif //MXSLC_MODIFIERLIST_H

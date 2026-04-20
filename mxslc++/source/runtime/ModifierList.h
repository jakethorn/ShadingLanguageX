//
// Created by jaket on 16/01/2026.
//

#ifndef MXSLC_MODIFIERLIST_H
#define MXSLC_MODIFIERLIST_H

#include "utils/common.h"
#include "TokenType.h"
#include "CompileError.h"

class ModifierList
{
public:
    ModifierList() = default;
    explicit ModifierList(vector<TokenType> mods);

    template<typename... Args>
    explicit ModifierList(Args&&... mods)
    {
        (add(std::forward<Args>(mods)), ...);
    }

    template<typename... Args>
    void validate(const Args&... valid_mods) const
    {
        for (const TokenType& mod : mods_)
        {
            if (not (... || (mod == valid_mods)))
                throw CompileError{"'" + mod.str() + "' is not a valid modifier here"};
        }
    }

    void add(const TokenType mod) { mods_.insert(mod); }
    void remove(const TokenType mod) { mods_.erase(mod); }

    template<typename... Args>
    ModifierList without(Args&&... mods) const
    {
        ModifierList list = *this;
        (list.remove(std::forward<Args>(mods)), ...);
        return list;
    }

    bool contains(const TokenType mod) const { return ::contains(mods_, mod.str()); }
    size_t size() const { return mods_.size(); }
    bool empty() const { return mods_.empty(); }

    string str() const;

private:
    unordered_set<TokenType> mods_;
};

#endif //MXSLC_MODIFIERLIST_H

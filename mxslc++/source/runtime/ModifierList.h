//
// Created by jaket on 16/01/2026.
//

#ifndef MXSLC_MODIFIERLIST_H
#define MXSLC_MODIFIERLIST_H

#include "utils/common.h"
#include "TokenType.h"

class ModifierList
{
public:
    ModifierList() = default;
    explicit ModifierList(vector<string> mods);

    template<typename... Args>
    explicit ModifierList(Args&&... args)
    {
        (add(std::forward<Args>(args)), ...);
    }

    template<typename... Args>
    void validate(const Args&... valid_mods) const
    {
        for (const string& mod : mods_)
        {
            if (not (... || (TokenType{valid_mods} == mod)))
                throw_error("'" + mod + "' is not a valid modifier here");
        }
    }

    void add(string mod) { mods_.push_back(std::move(mod)); }
    void add(const TokenType mod) { mods_.push_back(mod.str()); }

    bool contains(const TokenType mod) const { return ::contains(mods_, mod.str()); }
    size_t size() const { return mods_.size(); }
    bool empty() const { return mods_.empty(); }

    string str() const;

private:
    static void throw_error(const string& msg) ;

    vector<string> mods_;
};

#endif //MXSLC_MODIFIERLIST_H

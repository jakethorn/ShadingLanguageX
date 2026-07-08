//
// Created by jaket on 16/01/2026.
//

#ifndef MXSLC_MODIFIERLIST_H
#define MXSLC_MODIFIERLIST_H

#include "common.h"
#include "TokenType.h"
#include "utils/container_utils.h"
#include "errors/CompileError.h"

namespace mxslc::runtime
{
    class ModifierList
    {
    public:
        ModifierList() = default;
        ModifierList(TokenType::Enum mod);
        explicit ModifierList(const vector<Token>& mods);

        template<typename... Args>
        void validate(const Args&... valid_mods) const
        {
            for (const TokenType& mod : mods_)
            {
                if (not (... || (mod == valid_mods)))
                    throw CompileError{"'" + mod.str() + "' is not a valid modifier here"};
            }
        }

        template<typename... Args>
        ModifierList without(Args&&... mods) const
        {
            ModifierList list = *this;
            (list.remove(std::forward<Args>(mods)), ...);
            return list;
        }

        template<typename... Args>
        ModifierList only(Args&&... mods) const
        {
            ModifierList list;
            ((this->contains(mods) ? list.add(mods) : void()), ...);
            return list;
        }

        void add(const TokenType mod) { mods_.insert(mod); }
        void remove(const TokenType mod) { mods_.erase(mod); }
        bool contains(const TokenType mod) const { return mxslc::contains(mods_, mod); }
        size_t size() const { return mods_.size(); }
        bool empty() const { return mods_.empty(); }
        string str() const;

    private:
        unordered_set<TokenType> mods_;
    };
}

#endif //MXSLC_MODIFIERLIST_H

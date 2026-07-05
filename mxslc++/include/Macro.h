//
// Created by jaket on 02/07/2026.
//

#ifndef MXSLC_MACRO_H
#define MXSLC_MACRO_H

#include <string>
#include <vector>

#include "Token.h"

namespace mxslc
{
    class Macro
    {
    public:
        Macro(std::string name);
        Macro(std::string name, std::vector<Token> body);
        Macro(std::string name, const std::string& body);

        const std::string& name() const { return name_; }
        const std::vector<Token>& body() const { return body_; }

        bool operator==(const Macro& other) const noexcept { return name_ == other.name_; }
        bool operator==(const std::string& other) const noexcept { return name_ == other; }
        bool operator==(const Token& other) const noexcept { return name_ == other.lexeme(); }

    private:
        std::string name_;
        std::vector<Token> body_;
    };
}

namespace std
{
    template<>
    struct hash<mxslc::Macro>
    {
        size_t operator()(const mxslc::Macro& macro) const noexcept
        {
            return std::hash<std::string>{}(macro.name());
        }
    };
}

#endif //MXSLC_MACRO_H

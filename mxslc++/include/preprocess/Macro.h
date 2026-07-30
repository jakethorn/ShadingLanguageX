//
// Created by jaket on 02/07/2026.
//

#ifndef MXSLC_MACRO_H
#define MXSLC_MACRO_H

#include "common.h"
#include "Token.h"

namespace mxslc::preprocess
{
    class Macro
    {
    public:
        explicit Macro(string name);
        Macro(string name, const string& body);
        Macro(string name, vector<Token> body);

        const string& name() const { return name_; }
        const vector<Token>& body() const { return body_; }

        bool operator==(const Macro& other) const noexcept { return name_ == other.name_; }
        bool operator==(const string& other) const noexcept { return name_ == other; }
        bool operator==(const Token& other) const noexcept { return name_ == other.lexeme(); }

        bool operator!=(const Macro& other) const noexcept { return name_ != other.name_; }
        bool operator!=(const string& other) const noexcept { return name_ != other; }
        bool operator!=(const Token& other) const noexcept { return name_ != other.lexeme(); }

    private:
        void validate() const;

        string name_;
        vector<Token> body_;
    };
}

namespace std
{
    template<>
    struct hash<mxslc::preprocess::Macro>
    {
        size_t operator()(const mxslc::preprocess::Macro& macro) const noexcept
        {
            return std::hash<std::string>{}(macro.name());
        }
    };
}

#endif //MXSLC_MACRO_H

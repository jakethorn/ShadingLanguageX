//
// Created by jaket on 24/11/2025.
//

#ifndef FENNEC_TOKENTYPE_H
#define FENNEC_TOKENTYPE_H

#include "utils/common.h"
#include "utils/template_utils.h"

class TokenType
{
public:
    enum Enum : unsigned char
    {
        // Symbol
        LeftParen    = '(',
        RightParen   = ')',
        LeftBrace    = '{',
        RightBrace   = '}',
        LeftBracket  = '[',
        RightBracket = ']',
        Period       = '.',
        Comma        = ',',
        Colon        = ':',
        Semicolon    = ';',
        At           = '@',
        Bang         = '!',
        Equals       = '=',
        Greater      = '>',
        Less         = '<',
        Plus         = '+',
        Minus        = '-',
        Star         = '*',
        Slash        = '/',
        Percent      = '%',
        Caret        = '^',
        Ampersand    = '&',
        Pipe         = '|',

        // Compound Symbols
        BangEq = 128,
        EqualsEq,
        GreaterEq,  
        LessEq,     
        PlusEq,     
        MinusEq,    
        StarEq,     
        SlashEq,    
        PercentEq,  
        CaretEq,    
        AmpersandEq,
        PipeEq,
        Arrow,
        
        // Keyword
        If,
        Else,
        Switch,
        For,
        Return,
        Auto,
        Null,
        Out,
        Const,
        Mutable,
        Consteval,
        Global,
        Inline,
        Default,
        Function,
        Typedef,
        Using,
        Struct,
        Uniform,
        Varying,
        Namespace,
        Print,
        Break,

        // Dynamic
        Identifier,
        Bool,
        Int,
        Float,
        String,
        Whitespace,
        Newline,
        Comment,

        // Error
        Unknown
    };

    inline static const unordered_set Symbols {
        LeftParen, RightParen, LeftBrace, RightBrace, LeftBracket, RightBracket, Period, Comma, Colon, Semicolon, At,
        Bang, Equals, Greater, Less, Plus, Minus, Star, Slash, Percent, Caret, Ampersand, Pipe
    };

    inline static const unordered_set CompoundSymbols {
        BangEq, EqualsEq, GreaterEq, LessEq, PlusEq, MinusEq, StarEq, SlashEq, PercentEq, CaretEq, AmpersandEq, PipeEq,
        Arrow
    };

    inline static const unordered_set Keywords {
        If, Else, Switch, For, Return, Auto, Null, Out, Const, Mutable, Consteval, Global, Inline, Default,
        Function, Typedef, Using, Struct, Uniform, Varying, Namespace, Print, Break
    };

private:
    inline static const unordered_map<string, Enum> lexemes_ {
        {"!=", BangEq},
        {"==", EqualsEq},
        {">=", GreaterEq},
        {"<=", LessEq},
        {"+=", PlusEq},
        {"-=", MinusEq},
        {"*=", StarEq},
        {"/=", SlashEq},
        {"%=", PercentEq},
        {"^=", CaretEq},
        {"&=", AmpersandEq},
        {"|=", PipeEq},
        {"->", Arrow},
        {"if", If},
        {"else", Else},
        {"switch", Switch},
        {"for", For},
        {"return", Return},
        {"auto", Auto},
        {"null", Null},
        {"out", Out},
        {"const", Const},
        {"mutable", Mutable},
        {"consteval", Consteval},
        {"global", Global},
        {"inline", Inline},
        {"default", Default},
        {"function", Function},
        {"typedef", Typedef},
        {"using", Using},
        {"struct", Struct},
        {"uniform", Uniform},
        {"varying", Varying},
        {"namespace", Namespace},
        {"print", Print},
        {"break", Break},
        {"<identifier>", Identifier},
        {"<bool>", Bool},
        {"<int>", Int},
        {"<float>", Float},
        {"<string>", String},
        {"<whitespace>", Whitespace},
        {"<newline>", Newline},
        {"<comment>", Comment},
        {"<unknown>", Unknown}
    };

    static Enum to_enum_(const char c)
    {
        if (const auto e = static_cast<Enum>(c); contains(Symbols, e))
            return e;
        return Unknown;
    }

    static Enum to_enum_(const string& s)
    {
        if (s.size() == 1)
            return to_enum_(s[0]);
        if (contains(lexemes_, s))
            return lexemes_.at(s);
        return Unknown;
    }

public:
    TokenType() : enum_{Unknown} { }
    TokenType(const Enum e) : enum_{e} { }
    TokenType(const char c) : enum_{to_enum_(c)} { }
    TokenType(const string& s) : enum_{to_enum_(s)} { }

    [[nodiscard]] bool is_symbol() const { return contains(Symbols, enum_); }
    [[nodiscard]] bool is_compound_symbol() const { return contains(CompoundSymbols, enum_); }
    [[nodiscard]] bool is_keyword() const { return contains(Keywords, enum_); }

    [[nodiscard]] string str() const
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

    bool operator==(const char c) const { return enum_ == to_enum_(c); }
    bool operator==(const string& s) const { return enum_ == to_enum_(s); }
    bool operator==(const TokenType& t) const { return enum_ == t.enum_; }
    bool operator==(const Enum e) const { return enum_ == e; }

    [[nodiscard]] size_t index() const { return enum_; }

private:
    Enum enum_;
};

template<>
struct std::hash<TokenType>
{
    size_t operator()(const TokenType& type) const noexcept
    {
        return type.index();
    }
};

#endif //FENNEC_TOKENTYPE_H

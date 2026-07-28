//
// Created by jaket on 24/11/2025.
//

#ifndef FENNEC_TOKENTYPE_H
#define FENNEC_TOKENTYPE_H

#include "common.h"
#include "utils/Stringable.h"

namespace mxslc
{
    class TokenType : public Stringable
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
            Hash         = '#',

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
            FatArrow,
            DoubleColon,
            Increment,
            Decrement,
            DoubleAt,
            DoubleAmpersand,
            DoublePipe,

            // Keyword
            If,
            Else,
            For,
            From,
            To,
            Return,
            Null,
            Ref,
            Out,
            Const,
            Mutable,
            Consteval,
            Global,
            Geomprop,
            Inline,
            Default,
            Using,
            Class,
            This,
            Uniform,
            Varying,
            Namespace,
            Print,
            Typeof,
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
            Bang, Equals, Greater, Less, Plus, Minus, Star, Slash, Percent, Caret, Ampersand, Pipe, Hash
        };

        inline static const unordered_set CompoundSymbols {
            BangEq, EqualsEq, GreaterEq, LessEq, PlusEq, MinusEq, StarEq, SlashEq, PercentEq, CaretEq, AmpersandEq, PipeEq,
            Arrow, FatArrow, DoubleColon, Increment, Decrement, DoubleAt, DoubleAmpersand, DoublePipe
        };

        inline static const unordered_set Keywords {
            If, Else, For, From, To, Return, Null, Ref, Out, Const, Mutable, Consteval, Global, Geomprop, Inline, Default,
            Using, Class, This, Uniform, Varying, Namespace, Print, Typeof, Break
        };

    private:
        inline static const unordered_map<string, Enum> Lexemes {
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
            {"=>", FatArrow},
            {"::", DoubleColon},
            {"++", Increment},
            {"--", Decrement},
            {"@@", DoubleAt},
            {"&&", DoubleAmpersand},
            {"||", DoublePipe},
            {"if", If},
            {"else", Else},
            {"for", For},
            {"from", From},
            {"to", To},
            {"return", Return},
            {"null", Null},
            {"ref", Ref},
            {"out", Out},
            {"const", Const},
            {"mutable", Mutable},
            {"consteval", Consteval},
            {"global", Global},
            {"geomprop", Geomprop},
            {"inline", Inline},
            {"default", Default},
            {"using", Using},
            {"class", Class},
            {"this", This},
            {"uniform", Uniform},
            {"varying", Varying},
            {"namespace", Namespace},
            {"print", Print},
            {"typeof", Typeof},
            {"break", Break},
            {"<bool>", Bool},
            {"<int>", Int},
            {"<float>", Float},
            {"<string>", String},
            {"<whitespace>", Whitespace},
            {"<newline>", Newline},
            {"<comment>", Comment},
            {"<unknown>", Unknown}
        };

    public:
        TokenType() : enum_{Unknown} { }
        TokenType(const Enum e) : enum_{e} { }
        TokenType(const char c) : enum_{to_enum(c)} { }
        TokenType(const string& s) : enum_{to_enum(s)} { }

        bool is_symbol() const;
        bool is_compound_symbol() const;
        bool is_keyword() const;

        string to_string() const override;

        bool operator==(const char c) const { return enum_ == to_enum(c); }
        bool operator==(const string& s) const { return enum_ == to_enum(s); }
        bool operator==(const char* const s) const { return enum_ == to_enum(s); }
        bool operator==(const TokenType& t) const { return enum_ == t.enum_; }
        bool operator==(const Enum e) const { return enum_ == e; }

        bool operator!=(const char c) const { return enum_ != to_enum(c); }
        bool operator!=(const string& s) const { return enum_ != to_enum(s); }
        bool operator!=(const char* const s) const { return enum_ != to_enum(s); }
        bool operator!=(const TokenType& t) const { return enum_ != t.enum_; }
        bool operator!=(const Enum e) const { return enum_ != e; }

        friend bool operator==(const char c, const TokenType& t) { return t == c; }
        friend bool operator==(const string& s, const TokenType& t) { return t == s; }
        friend bool operator==(const char* const s, const TokenType& t) { return t == s; }
        friend bool operator==(const Enum e, const TokenType& t) { return t == e; }

        friend bool operator!=(const char c, const TokenType& t) { return t != c; }
        friend bool operator!=(const string& s, const TokenType& t) { return t != s; }
        friend bool operator!=(const char* const s, const TokenType& t) { return t != s; }
        friend bool operator!=(const Enum e, const TokenType& t) { return t != e; }

        size_t index() const { return enum_; }

    private:
        static Enum to_enum(char c);
        static Enum to_enum(const string& s);

        Enum enum_;
    };
}

namespace std
{
    template<>
    struct hash<mxslc::TokenType>
    {
        size_t operator()(const mxslc::TokenType& type) const noexcept
        {
            return type.index();
        }
    };
}

#endif //FENNEC_TOKENTYPE_H

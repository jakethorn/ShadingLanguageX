//
// Created by jaket on 26/07/2026.
//

#include "preprocess/parse_primitive.h"

#include "TokenReader.h"
#include "CompileOptions.h"
#include "scan.h"

namespace mxslc::preprocess
{
    Primitive parse_primitive(const string& code)
    {
        return parse_primitive(scan_string(code));
    }

    Primitive parse_primitive(vector<Token> tokens)
    {
        PrimitiveParser parser{std::move(tokens)};
        return parser.parse();
    }

    PrimitiveParser::PrimitiveParser(vector<Token> tokens) : TokenReader{std::move(tokens)}
    {

    }

    Primitive PrimitiveParser::parse()
    {
        return expression();
    }

    Primitive PrimitiveParser::expression()
    {
        return logical();
    }

    Primitive PrimitiveParser::logical()
    {
        Primitive lhs = equality();
        while (optional<Token> op = consume('&', '|'))
        {
            Primitive rhs = equality();
            lhs = evaluate(lhs, *op, rhs);
        }

        return lhs;
    }

    Primitive PrimitiveParser::equality()
    {
        Primitive lhs = relational();
        while (optional<Token> op = consume("==", "!="))
        {
            Primitive rhs = relational();
            lhs = evaluate(lhs, *op, rhs);
        }

        return lhs;
    }

    Primitive PrimitiveParser::relational()
    {
        Primitive lhs = term();
        if (optional<Token> op = consume('>', ">=", '<', "<="))
        {
            Primitive mhs = term();
            lhs = evaluate(lhs, *op, mhs);

            if (optional<Token> op2 = consume('>', ">=", '<', "<="))
            {
                Primitive rhs = term();
                lhs = lhs and evaluate(mhs, *op2, rhs);
            }
        }

        return lhs;
    }

    Primitive PrimitiveParser::term()
    {
        Primitive lhs = factor();
        while (optional<Token> op = consume('+', '-'))
        {
            Primitive rhs = factor();
            lhs = evaluate(lhs, *op, rhs);
        }

        return lhs;
    }

    Primitive PrimitiveParser::factor()
    {
        Primitive lhs = exponent();
        while (optional<Token> op = consume('*', '/', '%'))
        {
            Primitive rhs = exponent();
            lhs = evaluate(lhs, *op, rhs);
        }

        return lhs;
    }

    Primitive PrimitiveParser::exponent()
    {
        Primitive lhs = unary();
        while (optional<Token> op = consume('^'))
        {
            Primitive rhs = unary();
            lhs = evaluate(lhs, *op, rhs);
        }

        return lhs;
    }

    Primitive PrimitiveParser::unary()
    {
        if (optional<Token> op = consume('!', '+', '-'))
        {
            Primitive rhs = primary();
            return evaluate(*op, rhs);
        }

        return primary();
    }

    Primitive PrimitiveParser::primary()
    {
        if (optional<Token> token = consume(TokenType::Bool, TokenType::Int, TokenType::Float, TokenType::String))
        {
            return token->literal();
        }

        if (consume('('))
        {
            Primitive value = expression();
            match(')');
            return value;
        }

        throw CompileError{peek(), "Invalid preprocessor expression"};
    }

    Primitive PrimitiveParser::evaluate(const Primitive& lhs, const Token& op, const Primitive& rhs)
    {
        if (op == '+') return lhs + rhs;
        if (op == '-') return lhs - rhs;
        if (op == '*') return lhs * rhs;
        if (op == '/') return lhs / rhs;
        if (op == '%') return lhs % rhs;
        if (op == '^') return lhs ^ rhs;
        if (op == '>') return lhs > rhs;
        if (op == ">=") return lhs >= rhs;
        if (op == '<') return lhs < rhs;
        if (op == "<=") return lhs <= rhs;
        if (op == "==") return lhs == rhs;
        if (op == "!=") return lhs != rhs;
        if (op == '&') return lhs & rhs;
        if (op == '|') return lhs | rhs;
        throw CompileError{"Invalid primitive binary operator: " + op.lexeme()};
    }

    Primitive PrimitiveParser::evaluate(const Token& op, const Primitive& rhs)
    {
        if (op == '!') return !rhs;
        if (op == '+') return +rhs;
        if (op == '-') return -rhs;
        throw CompileError{"Invalid primitive unary operator: " + op.lexeme()};
    }
}

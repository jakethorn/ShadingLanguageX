//
// Created by jaket on 26/07/2026.
//

#ifndef MXSLC_PARSE_PRIMITIVE_H
#define MXSLC_PARSE_PRIMITIVE_H

#include "common.h"
#include "Primitive.h"
#include "Token.h"
#include "TokenReader.h"

namespace mxslc::preprocess
{
    Primitive parse_primitive(const string& code);
    Primitive parse_primitive(vector<Token> tokens);

    class PrimitiveParser final : protected TokenReader
    {
    public:
        explicit PrimitiveParser(vector<Token> tokens);

        Primitive parse();

    private:
        Primitive expression();
        Primitive logical();
        Primitive equality();
        Primitive relational();
        Primitive term();
        Primitive factor();
        Primitive exponent();
        Primitive unary();
        Primitive primary();

        Primitive evaluate(const Token& op, const Primitive& rhs);
        Primitive evaluate(const Primitive& lhs, const Token& op, const Primitive& rhs);
    };
}

#endif //MXSLC_PARSE_PRIMITIVE_H

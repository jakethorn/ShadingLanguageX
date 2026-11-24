//
// Created by jaket on 23/11/2026.
//

#ifndef MXSLC_DIAGNOSTIC_H
#define MXSLC_DIAGNOSTIC_H

#include "common.h"

namespace mxslc
{
    class Token;

    struct SourceLocation
    {
        fs::path path;
        size_t line{0};

        SourceLocation() = default;
        explicit SourceLocation(const Token& token);

        bool is_known() const { return line != 0; }
        string to_string() const;

        bool operator==(const SourceLocation& other) const;
    };

    struct Diagnostic
    {
        SourceLocation location;
        string message;

        string to_string() const;

        bool operator==(const Diagnostic& other) const;
    };
}

#endif //MXSLC_DIAGNOSTIC_H

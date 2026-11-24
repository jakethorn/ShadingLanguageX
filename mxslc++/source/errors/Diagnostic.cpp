//
// Created by jaket on 23/11/2026.
//

#include "errors/Diagnostic.h"
#include "Token.h"

namespace mxslc
{
    SourceLocation::SourceLocation(const Token& token) : path{token.path()}, line{token.line()}
    {

    }

    string SourceLocation::to_string() const
    {
        const string line_string = "line " + std::to_string(line);
        if (path.empty())
            return line_string;
        return path.filename().string() + ", " + line_string;
    }

    bool SourceLocation::operator==(const SourceLocation& other) const
    {
        return path == other.path and line == other.line;
    }

    string Diagnostic::to_string() const
    {
        if (location.is_known())
            return location.to_string() + ": " + message;
        return message;
    }

    bool Diagnostic::operator==(const Diagnostic& other) const
    {
        return location == other.location and message == other.message;
    }
}

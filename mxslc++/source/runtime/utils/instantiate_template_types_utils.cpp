//
// Created by jaket on 11/04/2026.
//

#include "runtime/utils/instantiate_template_types_utils.h"

#include "runtime/Type.h"

namespace mxslc::runtime::runtime_utils
{
    string instantiate_template_types(const string& src, const TypePtr& template_type)
    {
        return src == "T" ? template_type->name() : src;
    }

    Token instantiate_template_types(const Token& src, const TypePtr& template_type)
    {
        Token t{src.type(), instantiate_template_types(src.lexeme(), template_type)};
        t.set_line(src.line());
        t.set_filename(src.filename());
        return t;
    }
}

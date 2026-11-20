//
// Created by jaket on 19/11/2026.
//

#ifndef MXSLC_TYPEINFO_H
#define MXSLC_TYPEINFO_H

#include "common.h"
#include "FieldInfo.h"
#include "FunctionInfo.h"
#include "Source.h"

namespace mxslc::reflection
{
    struct TypeInfo
    {
        string name;
        bool is_alias{false};
        Source source{Source::UserDefined};
        string aliased_type;
        vector<FieldInfo> fields;
        vector<FunctionInfo> methods;
        fs::path path;
        size_t line{0};
    };
}

#endif //MXSLC_TYPEINFO_H

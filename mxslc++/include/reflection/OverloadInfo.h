//
// Created by jaket on 20/11/2026.
//

#ifndef MXSLC_OVERLOADINFO_H
#define MXSLC_OVERLOADINFO_H

#include "common.h"
#include "ParameterInfo.h"
#include "Source.h"

namespace mxslc::reflection
{
    struct OverloadInfo
    {
        vector<ParameterInfo> parameters;
        string return_type;
        string template_type;
        vector<string> modifiers;
        Source source{Source::UserDefined};
        string class_type;
        fs::path path;
        size_t line{0};
    };
}

#endif //MXSLC_OVERLOADINFO_H

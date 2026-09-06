//
// Created by jaket on 05/09/2026.
//

#ifndef MXSLC_POSTPROCESS_UTILS_H
#define MXSLC_POSTPROCESS_UTILS_H

#include "runtime/interface.h"

namespace mxslc
{
    class FunctionCallHistory;
}

namespace mxslc::serialize
{
    void convert_nodedef_to_nodegraph(const ConstFuncPtr& func);
    void convert_nodedefs_to_nodegraphs(const FunctionCallHistory& func_call_history);
}

#endif //MXSLC_POSTPROCESS_UTILS_H

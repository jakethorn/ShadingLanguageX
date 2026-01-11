//
// Created by jaket on 03/01/2026.
//

#include "mtlx_utils.h"
#include "utils/str_utils.h"

string as_string(const mx::NodePtr& node)
{
    string str = node->asString();
    for (const mx::InputPtr& i : node->getInputs())
        str += "\n    "s + i->asString();
    for (const mx::OutputPtr& i : node->getOutputs())
        str += "\n    "s + i->asString();
    return str;
}

string get_output_name(const size_t i)
{
    return "out" + str(i+1);
}

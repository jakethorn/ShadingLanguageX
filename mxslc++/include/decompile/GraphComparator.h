//
// Created by jaket on 28/09/2026.
//

#ifndef MXSLC_GRAPHCOMPARATOR_H
#define MXSLC_GRAPHCOMPARATOR_H

#include <MaterialXCore/Document.h>

#include "common.h"

namespace mxslc::decompile
{
    // Compares two documents structurally. Nodes are identified by what they compute (their category, type, input values
    // and the nodes they are connected to) and not by their names, so documents that only differ by the names or order of
    // their nodes are equivalent. Decompile hints are ignored.
    class GraphComparator
    {
    public:
        // returns the names of the elements that differ, empty if the documents are equivalent
        static vector<string> differences(const mx::DocumentPtr& a, const mx::DocumentPtr& b);
    };
}

#endif //MXSLC_GRAPHCOMPARATOR_H

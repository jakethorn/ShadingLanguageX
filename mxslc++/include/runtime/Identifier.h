//
// Created by jaket on 10/11/2026.
//

#ifndef MXSLC_IDENTIFIER_H
#define MXSLC_IDENTIFIER_H

#include "common.h"

namespace mxslc::runtime
{
    class Identifier
    {
    public:
        Identifier(string name);
        Identifier(vector<string> names);
        Identifier(NamespacePtr namespace_, string name);

    private:
        NamespacePtr namespace_;
        string name_;
    };
}

#endif //MXSLC_IDENTIFIER_H

//
// Created by jaket on 09/11/2026.
//

#ifndef MXSLC_NAMESPACE_H
#define MXSLC_NAMESPACE_H

#include "common.h"

namespace mxslc::runtime
{
    class Namespace
    {
    public:
        explicit Namespace(string name) : name_{std::move(name)}, parent_{nullptr} { }
        Namespace(string name, NamespacePtr parent) : name_{std::move(name)}, parent_{std::move(parent)} { }

        const string& name() const { return name_; }
        NamespacePtr parent() const { return parent_; }

    private:
        string name_;
        NamespacePtr parent_;
    };
}

#endif //MXSLC_NAMESPACE_H

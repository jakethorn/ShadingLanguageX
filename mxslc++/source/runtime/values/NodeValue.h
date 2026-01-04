//
// Created by jaket on 01/01/2026.
//

#ifndef FENNEC_NODEVALUE_H
#define FENNEC_NODEVALUE_H

#include <MaterialXCore/Node.h>
#include "utils/common.h"
#include "Value.h"
#include "runtime/Type.h"
#include "mtlx/mtlx_utils.h"

class NodeValue final : public Value
{
public:
    NodeValue(const mx::NodePtr& node) : node_{node}, type_{node_->getType()} { }

    [[nodiscard]] const Type& type() const override { return type_; }
    [[nodiscard]] string str() const override { return as_string(node_); }

    [[nodiscard]] const mx::NodePtr& node() const { return node_; }

private:
    mx::NodePtr node_;
    Type type_;
};

#endif //FENNEC_NODEVALUE_H

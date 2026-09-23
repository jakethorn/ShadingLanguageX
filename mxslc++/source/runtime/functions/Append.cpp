//
// Created by jaket on 19/09/2026.
//

#include "runtime/functions/Append.h"

#include "runtime/variables/Variable.h"
#include "runtime/ArgumentList.h"
#include "runtime/interface.h"
#include "runtime/Type.h"

namespace mxslc::runtime
{
    Append::Append(const VarPtr& instance, const ArgumentList& args) : BuiltinFunction{"append", instance, args}
    {

    }

    void Append::init()
    {
        for (const Argument& arg : args_)
            arg.init();
    }

    TypePtr Append::return_type() const
    {
        vector<TypePtr> fields;

        for (const Field& field : instance_->type()->fields())
            fields.push_back(field.type());

        for (const Argument& arg : args_)
            fields.push_back(arg.type());

        return create_type(fields);
    }

    VarPtr Append::invoke() const
    {
        vector<VarPtr> children = instance_->children();

        for (const Argument& arg : args_)
        {
            arg.init();
            VarPtr child = arg.evaluate();
            children.push_back(child);
        }

        return create_variable(children);
    }
}

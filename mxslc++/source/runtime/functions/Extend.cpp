//
// Created by jaket on 19/09/2026.
//

#include "runtime/functions/Extend.h"

#include "runtime/variables/Variable.h"
#include "runtime/ArgumentList.h"
#include "runtime/interface.h"
#include "runtime/Type.h"

namespace mxslc::runtime
{
    Extend::Extend(const VarPtr& instance, const ArgumentList& args) : BuiltinFunction{"extend", instance, args}
    {

    }

    void Extend::init()
    {
        for (const Argument& arg : args_)
            arg.init();
    }

    TypePtr Extend::return_type() const
    {
        vector<TypePtr> fields;

        for (const Field& field : instance_->type()->fields())
            fields.push_back(field.type());

        for (const Argument& arg : args_)
        {
            if (arg.type()->has_fields())
            {
                for (const Field& field : arg.type()->fields())
                    fields.push_back(field.type());
            }
            else
            {
                fields.push_back(arg.type());
            }
        }

        return create_type(fields);
    }

    VarPtr Extend::invoke() const
    {
        vector<VarPtr> children = instance_->children();

        for (const Argument& arg : args_)
        {
            VarPtr sibling = arg.evaluate();

            if (sibling->has_children())
            {
                for (const VarPtr& child : sibling->children())
                    children.push_back(child);
            }
            else
            {
                children.push_back(sibling);
            }
        }

        return create_variable(children);
    }
}

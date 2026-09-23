//
// Created by jaket on 19/09/2026.
//

#include "runtime/functions/Insert.h"

#include "runtime/variables/Variable.h"
#include "runtime/interface.h"
#include "runtime/Type.h"
#include "runtime/ArgumentList.h"

namespace mxslc::runtime
{
    Insert::Insert(const VarPtr& instance, const ArgumentList& args) : BuiltinFunction{"insert", instance, args}
    {
        if (args.size() != 2)
            throw CompileError{"Method 'insert' expects 2 arguments: insert(int index, any value)"};
    }

    void Insert::init()
    {
        args_[0].init(Type::Int);
        args_[1].init();
    }

    TypePtr Insert::return_type() const
    {
        const int index = args_[0].evaluate()->compile_time_value<int>();

        const vector<Field>& fields = instance_->type()->fields();

        vector<TypePtr> field_types;
        for (size_t i = 0; i < fields.size(); ++i)
        {
            if (i == index)
                field_types.push_back(args_[1].type());
            field_types.push_back(fields[i].type());
        }

        return create_type(field_types);
    }

    VarPtr Insert::invoke() const
    {
        const int index = args_[0].evaluate()->compile_time_value<int>();
        const VarPtr value = args_[1].evaluate();

        vector<VarPtr> children;
        for (size_t i = 0; i < instance_->child_count(); ++i)
        {
            if (i == index)
                children.push_back(value);
            children.push_back(instance_->child(i));
        }

        return create_variable(children);
    }
}

//
// Created by jaket on 19/09/2026.
//

#include "runtime/functions/Len.h"

#include "runtime/ArgumentList.h"
#include "runtime/variables/Variable.h"
#include "runtime/interface.h"
#include "runtime/Type.h"

namespace mxslc::runtime
{
    Len::Len(const VarPtr& instance, const ArgumentList& args) : BuiltinFunction{"len", instance, args}
    {
        if (args.size() != 0)
            throw CompileError{"Method 'len' expects 0 arguments"};
    }

    TypePtr Len::return_type() const
    {
        return Type::Int;
    }

    VarPtr Len::invoke() const
    {
        return create_variable(instance_->child_count());
    }
}

//
// Created by jaket on 10/04/2026.
//

#include "Statement.h"

#include "CompileError.h"
#include "runtime/Runtime.h"

void Statement::execute()
{
    try
    {
        if (not is_initialized_)
            init();
        execute_impl();
    }
    catch (const CompileError& e)
    {
        throw CompileError{token_, e};
    }
}

Scope& Statement::scope()
{
    return Runtime::get().scope();
}

MtlXSerializer& Statement::serializer()
{
    return Runtime::get().serializer();
}

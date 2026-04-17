//
// Created by jaket on 10/04/2026.
//

#include "Statement.h"

#include "CompileError.h"
#include "runtime/Runtime.h"

void Statement::execute() const
{
    try
    {
        execute_impl();
    }
    catch (const CompileError& e)
    {
        throw CompileError{token_, e};
    }
}

Runtime& Statement::runtime()
{
    return Runtime::get();
}

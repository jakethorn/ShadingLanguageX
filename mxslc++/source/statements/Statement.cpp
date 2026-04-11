//
// Created by jaket on 10/04/2026.
//

#include "Statement.h"

#include "CompileError.h"

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

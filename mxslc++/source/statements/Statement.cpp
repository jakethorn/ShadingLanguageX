//
// Created by jaket on 10/04/2026.
//

#include "statements/Statement.h"

#include "debug/Debugger.h"
#include "errors/CompileError.h"

namespace mxslc::statements
{
    using debug::Debugger;

    void Statement::execute()
    {
        try
        {
            if (Debugger::is_enabled())
                Debugger::get().next_statement(this);

            if (not is_initialized_)
            {
                init();
                is_initialized_ = true;
            }

            execute_impl();
        }
        catch (CompileError& e)
        {
            e.set_debug_info(token_);
            throw;
        }
    }
}

//
// Created by jaket on 10/04/2026.
//

#include "statements/Statement.h"

#include "debug/Debugger.h"
#include "errors/CompileError.h"
#include "errors/FatalError.h"
#include "errors/SuppressedError.h"
#include "runtime/Runtime.h"

namespace mxslc::statements
{
    using debug::Debugger;

    void Statement::execute()
    {
        if (not is_recovery_point())
        {
            execute_and_locate_errors();
            return;
        }

        const Runtime::Checkpoint checkpoint = runtime().checkpoint();
        try
        {
            execute_and_locate_errors();
        }
        catch (const FatalError&)
        {
            throw;
        }
        catch (const SuppressedError&)
        {
            runtime().restore(checkpoint);
            recover();
        }
        catch (const CompileError& e)
        {
            runtime().restore(checkpoint);
            error_reporter().report(e);
            has_failed_ = true;
            recover();
        }
    }

    void Statement::execute_and_locate_errors()
    {
        try
        {
            if (has_failed_)
                throw SuppressedError{};

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

    bool Statement::is_recovery_point() const
    {
        return SourceLocation{token_}.is_known();
    }
}

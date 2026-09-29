//
// Created by jaket on 10/04/2026.
//

#include "statements/Statement.h"

#include "debug/Debugger.h"
#include "errors/CompileError.h"
#include "serialize/Serializer.h"

namespace mxslc::statements
{
    using debug::Debugger;

    void Statement::execute()
    {
        try
        {
            if (Debugger::is_enabled())
                Debugger::get().next_statement(this);

            const StatementHintFrame hint_frame{serializer().hints(), *this};

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

    string Statement::with_attributes(const AttributeList& attrs, const string& statement)
    {
        if (attrs.empty())
            return statement;
        return attrs.to_string() + "\n" + statement;
    }

    string join_statements(const vector<StmtPtr>& statements)
    {
        string result;
        for (size_t i = 0; i < statements.size(); ++i)
        {
            if (i > 0)
            {
                const bool is_block_boundary = statements[i - 1]->is_block() or statements[i]->is_block();
                result += is_block_boundary ? "\n\n" : "\n";
            }
            result += statements[i]->to_string();
        }
        return result;
    }
}

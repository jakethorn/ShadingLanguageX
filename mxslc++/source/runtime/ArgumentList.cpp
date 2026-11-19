//
// Created by jaket on 03/01/2026.
//

#include "runtime/ArgumentList.h"

#include "runtime/Parameter.h"
#include "runtime/ParameterList.h"
#include "runtime/variables/Variable.h"
#include "runtime/utils/monomorphize.h"
#include "errors/CompileError.h"

namespace mxslc::runtime
{
    ArgumentList::ArgumentList(vector<Argument> args) : args_{std::move(args)}
    {
        validate_arguments();
    }

    ArgumentList::ArgumentList(const vector<ExprPtr>& exprs)
    {
        for (size_t i = 0; i < exprs.size(); ++i)
            args_.emplace_back(exprs[i], i);

        validate_arguments();
    }

    ArgumentList::ArgumentList(const vector<VarPtr>& values)
    {
        for (size_t i = 0; i < values.size(); ++i)
            args_.emplace_back(values[i], i);

        validate_arguments();
    }

    ArgumentList::ArgumentList(const vector<Primitive>& values)
    {
        for (size_t i = 0; i < values.size(); ++i)
            args_.emplace_back(values[i], i);

        validate_arguments();
    }

    ArgumentList::ArgumentList(const ExprPtr& expr)
    {
        args_.emplace_back(expr, 0);
    }

    ArgumentList::ArgumentList(const VarPtr& value)
    {
        args_.emplace_back(value, 0);
    }

    ArgumentList::ArgumentList(const ValuePtr& value)
    {
        args_.emplace_back(value, 0);
    }

    ArgumentList ArgumentList::monomorphize(const TypePtr& template_type) const
    {
        return runtime_utils::monomorphize(args_, template_type);
    }

    VarPtr ArgumentList::evaluate(const Parameter& param) const
    {
        VarPtr value;
        if (const Argument* arg = (*this)[param])
        {
            value = arg->evaluate();
            if (value->is_temporary() and param.is_out())
                throw CompileError{"Temporary variable being passed to ref or out parameter '" + param.name() + "'"};
        }
        else if (param.has_default_value())
        {
            value = param.evaluate();
        }
        else
        {
            throw CompileError{"Function call missing argument for parameter '" + param.name() + "'"};
        }

        return value;
    }

    ParameterValues ArgumentList::evaluate(const ParameterList& params) const
    {
        ParameterValues result;
        result.reserve(params.size());
        for (const Parameter& param : params)
            result.emplace_back(param, evaluate(param));
        return result;
    }

    const Argument* ArgumentList::operator[](const Parameter& param) const
    {
        for (const Argument& arg : args_)
        {
            if (arg.has_name())
            {
                if (arg.name() == param.name())
                    return &arg;
            }
            else
            {
                if (arg.index() == param.index())
                    return &arg;
            }
        }
        return nullptr;
    }

    void ArgumentList::validate_arguments() const
    {
        bool last_was_named = false;
        for (const Argument& arg : args_)
        {
            // ensure all positional arguments come before all named arguments
            if (last_was_named and arg.is_positional())
                throw CompileError{"Positional argument appears after a named argument"};

            last_was_named = arg.is_named();

            // ensure no two arguments have the same name
            for (const Argument& other_arg : args_)
            {
                if (&arg == &other_arg)
                    continue;
                if (arg.has_name() and arg.name() == other_arg.name())
                    throw CompileError{"Two or more arguments have the name '" + arg.name() + "'"};
            }
        }
    }

    string ArgumentList::to_string() const
    {
        return join(args_, ", ");
    }
}

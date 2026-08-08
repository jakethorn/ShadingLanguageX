//
// Created by jaket on 21/11/2025.
//

#include <cassert>

#include "expressions/Expression.h"

#include "errors/CompileError.h"
#include "runtime/Runtime.h"
#include "runtime/Type.h"
#include "runtime/variables/Variable.h"
#include "runtime/utils/type_cast.h"
#include "errors/AmbiguousFunctionError.h"

#define TRY_START try {
#define TRY_END } catch (CompileError& e) { e.set_debug_info(token_); throw; }

namespace mxslc::expressions
{
    Expression::Expression(Token token) : token_{std::move(token)} { }

    void Expression::init()
    {
        init(vector<TypePtr>{});
    }

    void Expression::init(const TypePtr& type)
    {
        init(vector<TypePtr>{type});
    }

    void Expression::init(const vector<TypePtr>& types)
    {
        TRY_START

        if (not try_init(types))
        {
            throw CompileError{error_message_};
        }

        TRY_END
    }

    bool Expression::try_init(const TypePtr& type)
    {
        return try_init(vector<TypePtr>{type});
    }

    bool Expression::try_init(const vector<TypePtr>& types)
    {
        TRY_START

        for (const TypePtr& type : types)
            assert(type->is_resolved());

        try
        {
            init_subexpressions(types);
            init_impl(types);
        }
        catch (const AmbiguousFunctionError& e)
        {
            error_message_ = e.what();
            is_initialized_ = false;
            return false;
        }

        if (types.empty() or (types.size() == 1 and types[0]->is_auto()))
        {
            is_initialized_ = true;
            return true;
        }

        const TypePtr type = type_impl();
        assert(type->is_resolved());

        target_type_ = type->find_unique_compatible(types);

        if (target_type_ == nullptr)
            error_message_ = "Cannot assign an expression of type " + type->to_string() + " to a variable or parameter of type " + type_utils::to_string(types);

        is_initialized_ = target_type_ != nullptr;
        return is_initialized_;

        TRY_END
    }

    void Expression::update()
    {
        init(type());
    }

    void Expression::reset()
    {
        is_initialized_ = false;
        target_type_ = nullptr;
    }

    TypePtr Expression::type() const
    {
        TRY_START

        assert(is_initialized_);
        if (target_type_)
            return target_type_;
        TypePtr type = type_impl();
        assert(type->is_resolved());
        return type;

        TRY_END
    }

    VarPtr Expression::evaluate() const
    {
        TRY_START

        assert(is_initialized_);

        const bool reduce_graph_cache = serializer().reduce_graph();
        if (is_comptime())
            serializer().set_reduce_graph(true);

        VarPtr value = evaluate_impl();

        serializer().set_reduce_graph(reduce_graph_cache);

        if (target_type_)
        {
            assert(value->type()->equals(type_impl()));
            return runtime_utils::type_cast(target_type_, value);
        }

        if (is_comptime() and not value->is_compile_time())
            throw CompileError{"Expression could not be evaluated at compile-time"};

        return value;

        TRY_END
    }

    void Expression::assign(const VarPtr& value) const
    {
        TRY_START

        evaluate()->copy(value);

        TRY_END
    }
}

#undef TRY_START
#undef TRY_END

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

#define TRY_START try {
#define TRY_END } catch (CompileError& e) { e.set_debug_info(token_); throw; }

namespace mxslc::expressions
{
    Expression::Expression(Token token) : token_{std::move(token)} { }

    void Expression::init()
    {
        init(vector<TypePtr>{});
    }

    void Expression::init(const TypePtr& target_type)
    {
        init(vector<TypePtr>{target_type});
    }

    void Expression::init(const vector<TypePtr>& target_types)
    {
        TRY_START

        if (not try_init(target_types))
            throw CompileError{error_message_};

        TRY_END
    }

    bool Expression::try_init(const TypePtr& target_type)
    {
        return try_init(vector<TypePtr>{target_type});
    }

    bool Expression::try_init(const vector<TypePtr>& target_types)
    {
        TRY_START

        for (const TypePtr& type : target_types)
            assert(type->is_resolved());

        init_subexpressions(target_types);
        init_impl(target_types);

        if (target_types.empty() or (target_types.size() == 1 and target_types[0]->is_auto()))
        {
            is_initialized_ = true;
            return true;
        }

        const TypePtr type = type_impl();
        assert(type->is_resolved());

        const bool success = update_target_type(type, target_types);

        if (not success)
            error_message_ = "Cannot assign an expression of type " + type->to_string() + " to a variable or parameter of type " + type_utils::to_string(target_types);

        is_initialized_ = success;
        return success;

        TRY_END
    }

    void Expression::update()
    {
        init(type());
    }

    void Expression::reset()
    {
        target_type_ = nullptr;
        is_initialized_ = false;
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

        serializer().begin_comptime(is_comptime());
        VarPtr value = evaluate_impl();
        serializer().end_comptime();

        if (is_comptime() and not value->is_compile_time())
            throw CompileError{"Expression could not be evaluated at compile-time"};

        if (target_type_)
        {
            assert(value->type()->equals(type_impl()));
            return runtime_utils::type_cast(target_type_, value);
        }
        else
        {
            return value;
        }

        TRY_END
    }

    void Expression::assign(const VarPtr& value) const
    {
        TRY_START

        evaluate()->copy(value);

        TRY_END
    }

    bool Expression::update_target_type(const TypePtr& type, const vector<TypePtr>& target_types)
    {
        target_type_ = nullptr;

        if (target_types.empty())
            return true;

        vector<TypePtr> compatibles;
        for (const TypePtr& target_type : target_types)
        {
            if (type->equals(target_type))
                return true;

            if (type->is_compatible_with(target_type))
            {
                if (target_type->is_auto() or target_type->is_tuple())
                    compatibles.push_back(type);
                else
                    compatibles.push_back(target_type);
            }
        }

        if (compatibles.size() == 1)
        {
            if (compatibles[0] != type)
                target_type_ = compatibles[0];
            return true;
        }

        return false;
    }
}

#undef TRY_START
#undef TRY_END

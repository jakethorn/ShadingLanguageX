//
// Created by jaket on 02/08/2026.
//

#include "utils/swizzle_utils.h"

#include "runtime/Type.h"
#include "runtime/utils/invoke.h"
#include "runtime/variables/Variable.h"

namespace mxslc::swizzle_utils
{
    using container_utils::contains;

    namespace
    {
        template <typename T>
        VarPtr evaluate_swizzle_impl(const T& value, const string& swizzle)
        {
            if (swizzle.size() == 1)
            {
                return runtime_utils::invoke_function("extract", ArgumentList{value, get_channel_index(swizzle[0])});
            }

            const VarPtr separated_values = runtime_utils::invoke_function("separate", value);

            vector<VarPtr> channels;
            channels.reserve(swizzle.size());
            for (const char c : swizzle)
            {
                const int i = get_channel_index(c);
                VarPtr channel = separated_values->child(i);
                channels.push_back(channel);
            }

            return runtime_utils::invoke_function(get_swizzle_type(swizzle), "combine", channels);
        }
    }

    VarPtr evaluate_swizzle(const ExprPtr& value_expr, const string& swizzle)
    {
        return evaluate_swizzle_impl(value_expr, swizzle);
    }

    VarPtr evaluate_swizzle(const VarPtr& value, const string& swizzle)
    {
        return evaluate_swizzle_impl(value, swizzle);
    }

    TypePtr get_swizzle_type(const string& swizzle)
    {
        if (contains(swizzle, 'x') or contains(swizzle, 'y') or contains(swizzle, 'z') or contains(swizzle, 'w'))
        {
            if (swizzle.size() == 1)
                return Type::Float;
            if (swizzle.size() == 2)
                return Type::Vec2;
            if (swizzle.size() == 3)
                return Type::Vec3;
            if (swizzle.size() == 4)
                return Type::Vec4;
        }

        if (contains(swizzle, 'r') or contains(swizzle, 'g') or contains(swizzle, 'b') or contains(swizzle, 'a'))
        {
            if (swizzle.size() == 1)
                return Type::Float;
            if (swizzle.size() == 3)
                return Type::Color3;
            if (swizzle.size() == 4)
                return Type::Color4;
        }

        throw CompileError{"Invalid swizzle: " + swizzle};
    }

    int get_channel_index(const char channel)
    {
        if (channel == 'x' or channel == 'r')
            return 0;
        if (channel == 'y' or channel == 'g')
            return 1;
        if (channel == 'z' or channel == 'b')
            return 2;
        if (channel == 'w' or channel == 'a')
            return 3;

        throw CompileError{"Invalid swizzle channel: "s + channel};
    }

    bool is_swizzle(const string& swizzle)
    {
        if (swizzle.empty() or swizzle.size() > 4)
            return false;

        bool is_vec = false;
        bool is_color = false;
        for (const char c : swizzle)
        {
            if (c == 'x' or c == 'y' or c == 'z' or c == 'w')
                is_vec = true;
            else if (c == 'r' or c == 'g' or c == 'b' or c == 'a')
                is_color = true;
            else
                return false;
        }

        if (is_vec and is_color)
            return false;

        return true;
    }

    bool is_valid_swizzle(const TypePtr& type, const string& swizzle)
    {
        if (swizzle.empty() or swizzle.size() > 4)
            return false;

        if (type->is<mx::Vector2>())
        {
            for (const char c : swizzle)
            {
                if (c != 'x' and c != 'y')
                    return false;
            }
            return true;
        }

        if (type->is<mx::Vector3>())
        {
            for (const char c : swizzle)
            {
                if (c != 'x' and c != 'y' and c != 'z')
                    return false;
            }
            return true;
        }

        if (type->is<mx::Vector4>())
        {
            for (const char c : swizzle)
            {
                if (c != 'x' and c != 'y' and c != 'z' and c != 'w')
                    return false;
            }
            return true;
        }

        if (type->is<mx::Color3>())
        {
            for (const char c : swizzle)
            {
                if (c != 'r' and c != 'g' and c != 'b')
                    return false;
            }
            return true;
        }

        if (type->is<mx::Color4>())
        {
            for (const char c : swizzle)
            {
                if (c != 'r' and c != 'g' and c != 'b' and c != 'a')
                    return false;
            }
            return true;
        }

        return false;
    }

    bool has_duplicate_channels(const string& swizzle)
    {
        vector<char> chars;
        for (const char c : swizzle)
        {
            if (contains(chars, c))
                return true;
            chars.push_back(c);
        }

        return false;
    }
}

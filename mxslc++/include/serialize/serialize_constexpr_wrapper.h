//
// Created by jaket on 15/07/2026.
//

#ifndef MXSLC_SERIALIZE_CONSTEXPR_WRAPPERS_H
#define MXSLC_SERIALIZE_CONSTEXPR_WRAPPERS_H

#include <functional>

#include "common.h"
#include "Primitive.h"
#include "runtime/interface.h"

namespace mxslc::serialize_utils
{
    using constexpr_func = std::function<VarPtr(const TypePtr&, const vector<Primitive>&)>;

    template<typename Func, std::size_t... Is>
    VarPtr call_unpacked(const Func& func, const vector<Primitive>& values, std::index_sequence<Is...>)
    {
        return create_variable(func(values[Is]...));
    }

    template<typename Func, std::size_t... Is>
    VarPtr call_unpacked(const Func& func, const TypePtr& type, const vector<Primitive>& values, std::index_sequence<Is...>)
    {
        return create_variable(func(values[Is]..., type));
    }

    template <typename... Ts>
    constexpr bool requires_type()
    {
        if constexpr (sizeof...(Ts) == 0)
            return false;
        else
            return std::is_same_v<std::decay_t<std::tuple_element_t<sizeof...(Ts) - 1, std::tuple<Ts...>>>, TypePtr>;
    }

    template<typename R, typename... Ts>
    constexpr_func wrap(R (*func)(Ts...))
    {
        return [func](const TypePtr& type, const vector<Primitive>& values) -> VarPtr
        {
            if constexpr (requires_type<Ts...>())
                return call_unpacked(func, type, values, std::make_index_sequence<sizeof...(Ts) - 1>{});
            else
                return call_unpacked(func, values, std::index_sequence_for<Ts...>{});
        };
    }

    constexpr_func wrap_creatematrix();
    constexpr_func wrap_conditional_node(const std::function<Primitive(const Primitive&, const Primitive&)>& pred);
    constexpr_func wrap_switch();
}

#endif //MXSLC_SERIALIZE_CONSTEXPR_WRAPPERS_H

//
// Created by jaket on 07/12/2025.
//

#ifndef FENNEC_TEMPLATE_UTILS_H
#define FENNEC_TEMPLATE_UTILS_H

#include <algorithm>

#include "utils/common.h"

template<typename Dst, typename Src>
vector<Dst> cast_vector(const vector<Src>& src)
{
    vector<Dst> dst;
    dst.reserve(src.size());
    for (const Src& b : src)
        dst.emplace_back(b);
    return dst;
}

template<typename T>
vector<shared_ptr<T>> lock(const vector<weak_ptr<T>>& src)
{
    vector<shared_ptr<T>> dst;
    dst.reserve(src.size());
    for (const weak_ptr<T>& t : src)
        dst.push_back(t.lock());
    return dst;
}

template<typename T>
optional<T> as_optional(const bool pred, const T& value)
{
    if (pred)
        return value;
    return std::nullopt;
}

template<typename T0, typename T1>
bool contains(const vector<T0>& ts, const T1& t)
{
    return std::find(ts.begin(), ts.end(), t) != ts.end();
}

template<typename TSet, typename T>
bool contains(const TSet& ts, const T& t)
{
    return ts.find(t) != ts.end();
}

#endif //FENNEC_TEMPLATE_UTILS_H

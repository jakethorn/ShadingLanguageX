//
// Created by jaket on 07/12/2025.
//

#ifndef FENNEC_TEMPLATE_UTILS_H
#define FENNEC_TEMPLATE_UTILS_H

#include "common.h"

template<typename T>
vector<shared_ptr<T>> lock(const vector<weak_ptr<T>>& src)
{
    vector<shared_ptr<T>> dst;
    dst.reserve(src.size());
    for (const std::weak_ptr<T>& t : src)
        dst.push_back(t.lock());
    return dst;
}

template<typename T0, typename T1>
auto position_of(const vector<T0>& ts, const T1& t)
{
    return std::find(ts.begin(), ts.end(), t);
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

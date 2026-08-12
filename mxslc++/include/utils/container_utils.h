//
// Created by jaket on 07/12/2025.
//

#ifndef FENNEC_CONTAINER_UTILS_H
#define FENNEC_CONTAINER_UTILS_H

#include "common.h"

namespace mxslc::container_utils
{
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

    bool contains(const string& str, char c);
    bool contains(const vector<TypePtr>& types, const TypePtr& type);

    template<typename T>
    void extend(vector<T>& dst, const vector<T>& src)
    {
        dst.insert(dst.end(), src.begin(), src.end());
    }

    template<typename T>
    void extend(vector<T>& dst, vector<T>&& src)
    {
        dst.insert(dst.end(), std::make_move_iterator(src.begin()), std::make_move_iterator(src.end()));
    }
}

#endif //FENNEC_CONTAINER_UTILS_H

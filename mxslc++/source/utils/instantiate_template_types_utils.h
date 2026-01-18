//
// Created by jaket on 18/01/2026.
//

#ifndef MXSLC_INSTANTIATE_TEMPLATE_TYPES_UTILS_H
#define MXSLC_INSTANTIATE_TEMPLATE_TYPES_UTILS_H

#include "common.h"

template<typename T>
optional<T> instantiate_template_types(const optional<T>& src, const TypeInfoPtr& template_type)
{
    if (src)
        return src->instantiate_template_types(template_type);
    return std::nullopt;
}

template<typename T>
vector<T> instantiate_template_types(const vector<T>& src, const TypeInfoPtr& template_type)
{
    vector<T> dst;
    dst.reserve(src.size());
    for (const T& t : src)
        dst.push_back(t.instantiate_template_types(template_type));
    return dst;
}

template<typename T>
vector<unique_ptr<T>> instantiate_template_types(const vector<unique_ptr<T>>& src, const TypeInfoPtr& template_type)
{
    vector<unique_ptr<T>> dst;
    dst.reserve(src.size());
    for (const unique_ptr<T>& t : src)
        dst.push_back(t->instantiate_template_types(template_type));
    return dst;
}

template<typename T>
vector<shared_ptr<T>> instantiate_template_types(const vector<shared_ptr<T>>& src, const TypeInfoPtr& template_type)
{
    vector<shared_ptr<T>> dst;
    dst.reserve(src.size());
    for (const shared_ptr<T>& t : src)
        dst.push_back(t->instantiate_template_types(template_type));
    return dst;
}

#endif //MXSLC_INSTANTIATE_TEMPLATE_TYPES_UTILS_H

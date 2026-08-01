//
// Created by jaket on 18/01/2026.
//

#ifndef MXSLC_MONOMORPHIZE_H
#define MXSLC_MONOMORPHIZE_H

#include "common.h"

namespace mxslc::runtime_utils
{
    template<typename T>
    class Monomorphizable
    {
    public:
        virtual ~Monomorphizable() = default;
        virtual T monomorphize(const TypePtr& template_type) const = 0;
    };

    string monomorphize(const string& src, const TypePtr& template_type);

    template<typename T>
    T monomorphize(const T& src, const TypePtr& template_type)
    {
        return src.monomorphize(template_type);
    }

    template<typename T>
    optional<T> monomorphize(const optional<T>& src, const TypePtr& template_type)
    {
        if (src)
            return src->monomorphize(template_type);
        return std::nullopt;
    }

    template<typename T>
    unique_ptr<T> monomorphize(const unique_ptr<T>& src, const TypePtr& template_type)
    {
        if (src)
            return src->monomorphize(template_type);
        return nullptr;
    }

    template<typename T>
    shared_ptr<T> monomorphize(const shared_ptr<T>& src, const TypePtr& template_type)
    {
        if (src)
            return src->monomorphize(template_type);
        return nullptr;
    }

    template<typename T>
    vector<T> monomorphize(const vector<T>& src, const TypePtr& template_type)
    {
        vector<T> dst;
        dst.reserve(src.size());
        for (const T& t : src)
            dst.push_back(monomorphize(t, template_type));
        return dst;
    }

    template<typename T>
    vector<unique_ptr<T>> monomorphize(const vector<unique_ptr<T>>& src, const TypePtr& template_type)
    {
        vector<unique_ptr<T>> dst;
        dst.reserve(src.size());
        for (const unique_ptr<T>& t : src)
            dst.push_back(monomorphize(t, template_type));
        return dst;
    }

    template<typename T>
    vector<shared_ptr<T>> monomorphize(const vector<shared_ptr<T>>& src, const TypePtr& template_type)
    {
        vector<shared_ptr<T>> dst;
        dst.reserve(src.size());
        for (const shared_ptr<T>& t : src)
            dst.push_back(monomorphize(t, template_type));
        return dst;
    }

    template <typename... Args>
    auto monomorphize_all(const TypePtr& template_type, const Args&... args)
    {
        return std::make_tuple(monomorphize(args, template_type)...);
    }
}

#endif //MXSLC_MONOMORPHIZE_H

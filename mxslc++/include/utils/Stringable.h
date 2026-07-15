//
// Created by jaket on 07/07/2026.
//

#ifndef MXSLC_STRINGABLE_H
#define MXSLC_STRINGABLE_H

#include "common.h"

namespace mxslc
{
    class Stringable
    {
    public:
        Stringable() = default;
        virtual ~Stringable() = default;

        virtual string to_string() const = 0;

        template<typename Container>
        static string join(const Container& items, const string& delimiter)
        {
            string result;

            for (auto it = std::begin(items); it != std::end(items); ++it)
            {
                if (it != std::begin(items))
                    result += delimiter;
                result += to_string(*it);
            }

            return result;
        }

    private:
        template<typename T> static string to_string(const T& obj) { return obj.to_string(); }
        template<typename T> static string to_string(const T* obj) { return obj->to_string(); }
        template<typename T> static string to_string(const unique_ptr<T>& ptr) { return ptr->to_string(); }
        template<typename T> static string to_string(const shared_ptr<T>& ptr) { return ptr->to_string(); }
    };
}

#endif //MXSLC_STRINGABLE_H

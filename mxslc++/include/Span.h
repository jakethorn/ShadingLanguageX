//
// Created by jaket on 16/11/2025.
//

#ifndef MXSLC_SPAN_H
#define MXSLC_SPAN_H

#include <stdexcept>
#include <vector>
#include "utils/str_utils.h"

namespace mxslc
{
    template<typename T>
    class Span
    {
    public:
        explicit Span(const std::vector<T>& data) : Span{data, 0, data.size()} { }
        Span(const std::vector<T>& data, const size_t start) : Span{data, start, data.size() - start} { }
        Span(const std::vector<T>& data, const size_t start, const size_t size) : ptr_{data.data() + start}, size_{size}
        {
            if (start > data.size())
                throw std::out_of_range{"Start index (" + str(start) + ") is out of bounds for vector of size (" + str(data.size()) + ")"};
            if (start + size > data.size())
                throw std::out_of_range{"End index (" + str(start + size) + ") is out of bounds for vector of size (" + str(data.size()) + ")"};
        }

        Span(const T* data, const size_t size) : Span{data, 0, size} { }
        Span(const T* data, const size_t start, const size_t size) : ptr_{data + start}, size_{size} { }

        const T& operator[](std::ptrdiff_t i) const
        {
            if (i < 0)
                i = size_ + i;
            if (i >= 0 and i < size_)
                return *(ptr_ + i);
            throw std::out_of_range{"Index (" + str(i) + ") is out of bounds for span of size (" + str(size_) + ")"};
        }

        const T* begin() const
        {
            return ptr_;
        }

        const T* end() const
        {
            return ptr_ + size_;
        }

        [[nodiscard]] size_t size() const { return size_; }
        [[nodiscard]] bool empty() const { return size_ == 0; }
        [[nodiscard]] const T& front() const { return *ptr_; }
        [[nodiscard]] const T& back() const { return *(ptr_ + size_ - 1); }

        const T& pop_front()
        {
            if (empty())
                throw std::out_of_range{"Cannot pop from an empty span"};

            const T& t = front();
            ++ptr_;
            --size_;
            return t;
        }

        const T& pop_back()
        {
            if (empty())
                throw std::out_of_range{"Cannot pop from an empty span"};

            const T& t = back();
            --size_;
            return t;
        }

    private:
        const T* ptr_;
        size_t size_;
    };
}

#endif //MXSLC_SPAN_H

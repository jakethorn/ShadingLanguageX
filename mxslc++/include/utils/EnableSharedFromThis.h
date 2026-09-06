//
// Created by jaket on 05/09/2026.
//

#ifndef MXSLC_ENABLESHAREDFROMTHIS_H
#define MXSLC_ENABLESHAREDFROMTHIS_H

#include "common.h"

namespace mxslc
{
    template <typename T>
    class EnableSharedFromThis : public std::enable_shared_from_this<T>
    {
    public:
        virtual ~EnableSharedFromThis() = default;

        template <typename S>
        shared_ptr<S> shared_from_child()
        {
            return std::static_pointer_cast<S>(this->shared_from_this());
        }

        template <typename S>
        shared_ptr<const S> shared_from_child() const
        {
            return std::static_pointer_cast<const S>(this->shared_from_this());
        }
    };
}

#endif //MXSLC_ENABLESHAREDFROMTHIS_H

//
// Created by jaket on 08/09/2026.
//

#ifndef MXSLC_MATERIALXVALIDATEERROR_H
#define MXSLC_MATERIALXVALIDATEERROR_H

#include <exception>

#include "common.h"

namespace mxslc
{
    class MaterialXValidateError : public std::runtime_error
    {
    public:
        explicit MaterialXValidateError(const string& message) : std::runtime_error{format_message(message)} { }

    private:
        static string format_message(const string& message)
        {
            return message +
                "\nAttention! mxslc produced an invalid MaterialX document."
                "\nPlease copy the above output along with your code and open a new issue here: github.com/jakethorn/ShadingLanguageX/issues";
        }
    };
}

#endif //MXSLC_MATERIALXVALIDATEERROR_H

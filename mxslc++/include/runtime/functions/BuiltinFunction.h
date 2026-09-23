//
// Created by jaket on 19/09/2026.
//

#ifndef MXSLC_BUILTINFUNCTION_H
#define MXSLC_BUILTINFUNCTION_H

#include "runtime/Function.h"

namespace mxslc::runtime
{
    class ArgumentList;

    class BuiltinFunction : public Function
    {
    public:
        BuiltinFunction(string name, const VarPtr& instance, const ArgumentList& args);

        bool is_builtin() const override { return true; }

        void init() override { }
        TypePtr return_type() const override = 0;
        VarPtr invoke() const override = 0;

    protected:
        const VarPtr& instance_;
        const ArgumentList& args_;

    private:
        static ParameterList parameters(const ArgumentList& args);
    };
}

#endif //MXSLC_BUILTINFUNCTION_H

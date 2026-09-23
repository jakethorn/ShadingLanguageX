//
// Created by jaket on 19/09/2026.
//

#include "runtime/functions/BuiltinFunction.h"

#include "runtime/variables/Variable.h"
#include "runtime/ArgumentList.h"
#include "runtime/interface.h"

namespace mxslc::runtime
{
    BuiltinFunction::BuiltinFunction(string name, const VarPtr& instance, const ArgumentList& args)
        : Function{ModifierList{}, nullptr, std::move(name), nullptr, parameters(args)}, instance_{instance}, args_{args}
    {
        BuiltinFunction::init();
    }

    ParameterList BuiltinFunction::parameters(const ArgumentList& args)
    {
        vector<Parameter> params;
        for (size_t i = 0; i < args.size(); ++i)
        {
            string name = "in" + std::to_string(i);
            params.emplace_back(AttributeList{}, ModifierList{}, args[i].type(), std::move(name), nullptr, i);
        }

        return params;
    }
}

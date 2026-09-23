//
// Created by jaket on 19/09/2026.
//

#include "runtime/utils/tuple_methods.h"

#include "runtime/ArgumentList.h"
#include "runtime/Type.h"
#include "runtime/variables/Variable.h"

namespace mxslc::runtime_utils
{
    namespace
    {
        string get_return_type(const VarPtr& instance, const ArgumentList& args)
        {
            vector<TypePtr> types;
            types.push_back(instance->type());
            for (const Argument& arg : args)
                types.push_back(arg.type());

            string return_type{'{'};

            for (const TypePtr& type : types)
            {
                string type_str = type->to_string();
                if (type_str.front() == '{')
                    type_str = type_str.substr(1, type_str.size() - 2);
                return_type += type_str + ", ";
            }

            return_type += '}';

            return return_type;
        }
    }

    FuncPtr create_tuple_method(const VarPtr& instance, const string& method_name, const ArgumentList& args)
    {

    }
}

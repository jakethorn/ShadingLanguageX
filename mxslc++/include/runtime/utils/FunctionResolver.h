//
// Created by jaket on 27/06/2026.
//

#ifndef MXSLC_FUNCTIONRESOLVER_H
#define MXSLC_FUNCTIONRESOLVER_H

#include "RuntimeAware.h"
#include "runtime/FunctionQuery.h"

namespace mxslc::runtime
{
    class Argument;
    class ArgumentList;
}

namespace mxslc::runtime_utils
{
    using runtime_utils::RuntimeAware;

    FuncPtr resolve_function(const vector<TypePtr>& types, const string& name, const TypePtr& template_type, ArgumentList& args, const bool& is_argumentless);
    FuncPtr resolve_method(const TypePtr& class_type, const vector<TypePtr>& types, const string& name, const TypePtr& template_type, ArgumentList& args, const bool& is_argumentless);

    class FunctionResolver : protected RuntimeAware
    {
    public:
        explicit FunctionResolver(FunctionQuery query, TypePtr class_type, ArgumentList& args)
            : query_{std::move(query)}, class_type_{std::move(class_type)}, args_{args} { }

        FuncPtr resolve() const;

    private:
        FuncPtr resolve(const Scope*& scope, vector<FuncPtr>& checked_funcs) const;

        void reset_arguments() const;
        size_t init_arguments(const FuncPtr& func) const;
        size_t init_arguments(const vector<FuncPtr>& funcs) const;
        vector<TypePtr> get_parameter_types(const vector<FuncPtr>& funcs, const Argument& arg) const;
        FuncPtr get_default_function(const vector<FuncPtr>& funcs) const;
        void implicitly_cast_literals(const FuncPtr& func) const;

        FunctionQuery query_;
        TypePtr class_type_;
        ArgumentList& args_;

        mutable vector<string> underlying_errors_;
    };
}

#endif //MXSLC_FUNCTIONRESOLVER_H

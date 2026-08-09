//
// Created by jaket on 27/06/2026.
//

#ifndef MXSLC_FUNCTIONINITIALIZER_H
#define MXSLC_FUNCTIONINITIALIZER_H

#include <utility>

#include "RuntimeAware.h"
#include "runtime/FunctionQuery.h"

namespace mxslc::runtime
{
    class Argument;
}

namespace mxslc::runtime_utils
{
    using runtime_utils::RuntimeAware;

    FuncPtr resolve_function(const vector<TypePtr>& types, const string& name, const TypePtr& template_type, ArgumentList& args, const bool& is_argumentless);
    FuncPtr resolve_method(const TypePtr& class_type, const vector<TypePtr>& types, const string& name, const TypePtr& template_type, ArgumentList& args, const bool& is_argumentless);

    class FunctionInitializer : protected RuntimeAware
    {
    public:
        explicit FunctionInitializer(FunctionQuery query, TypePtr class_type, ArgumentList& args)
            : query_{std::move(query)}, class_type_{std::move(class_type)}, args_{args} { }

        FuncPtr initialize_function() const;
        FuncPtr initialize_method() const;

    private:
        FuncPtr try_init(const Scope*& scope, vector<FuncPtr>& checked_funcs) const;

        void reset_arguments() const;
        size_t try_init(const vector<FuncPtr>& funcs) const;
        size_t try_init_with_default(const vector<FuncPtr>& funcs) const;
        vector<TypePtr> get_parameter_types(const vector<FuncPtr>& funcs, const Argument& arg) const;

        FunctionQuery query_;
        TypePtr class_type_;
        ArgumentList& args_;

        mutable vector<string> underlying_errors_;
    };
}

#endif //MXSLC_FUNCTIONINITIALIZER_H

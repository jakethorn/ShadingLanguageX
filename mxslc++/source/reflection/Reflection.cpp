//
// Created by jaket on 04/11/2026.
//

#include "reflection/Reflection.h"

#include <algorithm>

#include "TokenType.h"
#include "runtime/Scope.h"
#include "runtime/utils/TypeName.h"
#include "utils/container_utils.h"
#include "utils/string_utils.h"

namespace mxslc::reflection
{
    using container_utils::extend;
    using runtime_utils::TypeName;
    using string_utils::starts_with;

    namespace
    {
        void sort_and_remove_duplicates(vector<string>& names)
        {
            std::sort(names.begin(), names.end());
            names.erase(std::unique(names.begin(), names.end()), names.end());
        }

        bool is_callable_function_name(const string& name)
        {
            return not TokenType{name}.is_keyword() and not starts_with(name, "__");
        }
    }

    void Reflection::add_global(string name, TypePtr type, VarPtr value)
    {
        // globals in different scopes are the same global
        for (const GlobalInfo& global : globals_)
            if (global.name == name)
                return;
        globals_.push_back(GlobalInfo{std::move(name), std::move(type), std::move(value)});
    }

    void Reflection::collect_info()
    {
        collect_type_names();
        collect_function_names();
    }

    void Reflection::collect_type_names()
    {
        type_names_ = scope().get_all_type_names();
        extend(type_names_, {TypeName::Auto, TypeName::Void, TypeName::Template});
        sort_and_remove_duplicates(type_names_);
    }

    void Reflection::collect_function_names()
    {
        function_names_.clear();
        for (string& name : scope().get_all_function_names())
        {
            if (is_callable_function_name(name))
                function_names_.push_back(std::move(name));
        }

        sort_and_remove_duplicates(function_names_);
    }
}

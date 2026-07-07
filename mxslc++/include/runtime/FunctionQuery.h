//
// Created by jaket on 06/05/2026.
//

#ifndef MXSLC_FUNCTIONQUERY_H
#define MXSLC_FUNCTIONQUERY_H

#include "runtime/ArgumentList.h"
#include "utils/common.h"

class FunctionQuery
{
public:
    struct Result
    {
        enum Status
        {
            Ignore = 0,     // The query should ignore this part of the function signature
            NoMatch = 1,    // The function did not match the query
            Default = 2,
            Match = 3       // The function matched the query
        };

        Status status = Default;

        // How well the function matched the query
        size_t score = 0;

        Result(const bool result) : status{result ? Match : NoMatch} { }
        Result(const Status status) : status{status} { }
        Result(const Status status, const size_t score) : status{status}, score{score} { }

        Result operator&(const Result& other) const
        {
            if (status == Ignore)
                return other;
            if (other.status == Ignore)
                return *this;
            if (status == NoMatch)
                return *this;
            if (other.status == NoMatch)
                return other;
            return Result{Match, score + other.score};
        }

        Result& operator&=(const Result & result)
        {
            *this = *this & result;
            return *this;
        }

        Result operator|(const Result& other) const
        {
            if (status == Ignore)
                return other;
            if (other.status == Ignore)
                return *this;
            if (status == NoMatch)
                return other;
            if (other.status == NoMatch)
                return *this;
            return Result{Match, std::max(score, other.score)};
        }

        Result& operator|=(const Result & result)
        {
            *this = *this | result;
            return *this;
        }

        bool operator==(const Result& other) const
        {
            return status == other.status and score == other.score;
        }

        bool operator>(const Result& other) const
        {
            if (status == other.status)
                return score > other.score;
            else
                return status > other.status;
        }

        bool operator<(const Result& other) const
        {
            if (status == other.status)
                return score < other.score;
            else
                return status < other.status;
        }
    };

    FunctionQuery() = default;

    explicit FunctionQuery(const string& name);
    FunctionQuery(const string& name, const TypePtr& template_type);

    FunctionQuery(const vector<TypePtr>& return_types, const string& name, const bool& is_parameterless);
    FunctionQuery(const vector<TypePtr>& return_types, const string& name, const TypePtr& template_type, const ArgumentList& args, const bool& is_parameterless);

    FunctionQuery(const TypePtr& class_type, const vector<TypePtr>& return_types, const string& name, const bool& is_parameterless);
    FunctionQuery(
        const TypePtr& class_type,
        const vector<TypePtr>& return_types,
        const string& name,
        const TypePtr& template_type,
        const ArgumentList& args,
        const bool& is_parameterless
    );

    // non-owning pointers
    const TypePtr* class_type = nullptr;
    const vector<TypePtr>* return_types = nullptr;
    const string* name = nullptr;
    const TypePtr* template_type = nullptr;
    const ArgumentList* args = nullptr;
    const bool* is_parameterless = nullptr;

    Result is_match(const FuncPtr& func) const;
    Result has_match(const vector<FuncPtr>& funcs) const;
    FuncPtr get_match(const vector<FuncPtr>& funcs, bool throw_on_fail) const;
    vector<FuncPtr> get_matches(const vector<FuncPtr>& funcs) const;

private:
    Result class_type_matches(const FuncPtr& func) const;
    Result return_type_matches(const FuncPtr& func) const;
    Result name_matches(const FuncPtr& func) const;
    Result template_type_matches(const FuncPtr& func) const;
    Result arguments_match(const FuncPtr& func) const;
    Result is_parameterless_matches(const FuncPtr& func) const;

    vector<FuncPtr> get_default_functions(const vector<FuncPtr>& funcs) const;
};

#endif //MXSLC_FUNCTIONQUERY_H

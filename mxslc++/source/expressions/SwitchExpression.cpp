//
// Created by jaket on 26/10/2026.
//

#include "expressions/SwitchExpression.h"

#include "expressions/interface.h"
#include "preprocess/parse_primitive.h"
#include "runtime/ArgumentList.h"
#include "runtime/interface.h"
#include "runtime/Type.h"
#include "runtime/variables/Variable.h"
#include "runtime/utils/invoke.h"
#include "utils/container_utils.h"
#include "utils/primitive_utils.h"

namespace mxslc::expressions
{
    using namespace container_utils;
    using primitive_utils::floor;

    namespace
    {
        // switch nodes have 10 inputs, so to support more indices, an outer switch node selects between inner switch
        // nodes, which supports up to 100 indices
        constexpr size_t SwitchInputCount = 10;
        constexpr size_t SwitchIndexCount = SwitchInputCount * SwitchInputCount;
    }

    SwitchCase SwitchCase::monomorphize(const TypePtr& template_type) const
    {
        return SwitchCase{predicate_, is_default_, runtime_utils::monomorphize(value_expr_, template_type)};
    }

    string SwitchCase::to_string() const
    {
        if (is_default())
            return "default: " + value_expr_->to_string();
        if (has_predicate())
            return "case " + join(predicate_) + ": " + value_expr_->to_string();
        return value_expr_->to_string();
    }

    SwitchExpression::SwitchExpression(ExprPtr which_expr, vector<SwitchCase> cases, Token token)
        : Expression{std::move(token)}, which_expr_{std::move(which_expr)}, cases_{std::move(cases)}
    {

    }

    ExprPtr SwitchExpression::monomorphize(const TypePtr& template_type) const
    {
        return create_expression<SwitchExpression>(
            runtime_utils::monomorphize(which_expr_, template_type),
            runtime_utils::monomorphize(cases_, template_type),
            token_
        );
    }

    void SwitchExpression::init_subexpressions(const vector<TypePtr>& types)
    {
        which_expr_->init({Type::Int, Type::Float});

        for (const SwitchCase& case_ : cases_)
            case_.value_expr()->init(types);

        const TypePtr type = cases_.front().value_expr()->type();
        for (const SwitchCase& case_ : cases_)
        {
            if (not case_.value_expr()->type()->equals(type, /*field_names*/false))
                throw CompileError{"Switch cases do not evaluate to the same type"};
        }
    }

    void SwitchExpression::init_impl(const vector<TypePtr>& types)
    {
        index_case_expressions();
    }

    TypePtr SwitchExpression::type_impl() const
    {
        return cases_.front().value_expr()->type();
    }

    VarPtr SwitchExpression::evaluate_impl() const
    {
        const VarPtr which = which_expr_->evaluate();

        if (which->is_compile_time())
        {
            const int index = floor<int>(which->compile_time_value());

            if (index < 0 or index >= static_cast<int>(SwitchIndexCount))
                throw CompileError{"Switch which index " + std::to_string(index) + " is out of range, indices must be between 0 and " + std::to_string(SwitchIndexCount - 1)};

            if (contains(indexed_case_exprs_, index))
                return indexed_case_exprs_.at(index)->evaluate();
            return create_variable(type());
        }

        return runtime_evaluate(which);
    }

    namespace
    {
        // true for cases that compare against the switch value, e.g., case <=4
        bool is_relational(const SwitchCase& case_)
        {
            if (not case_.has_predicate())
                return false;

            const Token& op = case_.predicate().front();
            return op == '<' or op == '>' or op == TokenType::LessEq or op == TokenType::GreaterEq or op == TokenType::BangEq;
        }

        // the index selected by a case without a predicate, or with a predicate of the form n or ==n
        int get_explicit_index(const SwitchCase& case_, const size_t position)
        {
            if (not case_.has_predicate())
                return static_cast<int>(position);

            vector<Token> tokens = case_.predicate();
            if (tokens.front() == TokenType::EqualsEq)
                tokens.erase(tokens.begin());

            const Primitive index = parse_primitive(std::move(tokens));
            if (not index.is_a<int>())
                throw CompileError{case_.predicate().front(), "Switch case index must be an integer"};

            return index.as<int>();
        }

        bool is_match(const SwitchCase& case_, const size_t index)
        {
            vector<Token> tokens{std::to_string(index)};
            extend(tokens, case_.predicate());

            const Primitive result = parse_primitive(std::move(tokens));
            if (not result.is_a<bool>())
                throw CompileError{case_.predicate().front(), "Invalid switch case predicate"};

            return result.as<bool>();
        }
    }

    void SwitchExpression::index_case_expressions()
    {
        indexed_case_exprs_.clear();

        // cases are processed in order, replacing the matches of earlier cases, so later cases take precedence
        for (size_t position = 0; position < cases_.size(); ++position)
        {
            const SwitchCase& case_ = cases_[position];
            if (case_.is_default())
                continue;

            if (is_relational(case_))
            {
                for (size_t index = 0; index < SwitchIndexCount; ++index)
                {
                    if (is_match(case_, index))
                        indexed_case_exprs_[index] = case_.value_expr();
                }
            }
            else
            {
                const int index = get_explicit_index(case_, position);
                if (index < 0 or index >= static_cast<int>(SwitchIndexCount))
                    throw CompileError{"Switch case index " + std::to_string(index) + " is out of range, indices must be between 0 and " + std::to_string(SwitchIndexCount - 1)};

                indexed_case_exprs_[index] = case_.value_expr();
            }
        }

        // the default case selects every remaining index
        ExprPtr default_expr;
        for (const SwitchCase& case_ : cases_)
        {
            if (not case_.is_default())
                continue;

            if (default_expr)
                throw CompileError{"Switch expression has multiple default cases"};

            default_expr = case_.value_expr();
        }

        if (default_expr)
        {
            for (size_t index = 0; index < SwitchIndexCount; ++index)
                indexed_case_exprs_.try_emplace(index, default_expr);
        }
    }

    namespace
    {
        // true if every input is connected to the same value, in which case a switch node isn't needed
        bool all_equal(const Indexed<VarPtr>& inputs)
        {
            return inputs.size() == SwitchInputCount and
                std::all_of(inputs.begin(), inputs.end(), [&](const auto& input) { return input.second == inputs.begin()->second; });
        }

        // inputs maps each switch node input to its value, absent inputs are left unconnected
        VarPtr create_switch_node(const Indexed<VarPtr>& inputs, const VarPtr& which)
        {
            const VarPtr& first_input = inputs.begin()->second;
            if (first_input->type()->is_primitive())
            {
                ArgumentList args;
                for (const auto& [i, input] : inputs)
                    args.add("in" + std::to_string(i + 1), as_expression(input));
                args.add("which", as_expression(which));

                return runtime_utils::invoke_function("switch", std::move(args));
            }
            else
            {
                vector<VarPtr> children;
                children.reserve(first_input->child_count());
                for (size_t field = 0; field < first_input->child_count(); ++field)
                {
                    Indexed<VarPtr> field_inputs;
                    for (const auto& [i, input] : inputs)
                        field_inputs.emplace(i, input->child(field));

                    children.push_back(create_switch_node(field_inputs, which));
                }

                return create_variable(children);
            }
        }
    }

    VarPtr SwitchExpression::runtime_evaluate(const VarPtr& which) const
    {
        // evaluate each case once, in the order they are written
        unordered_map<ExprPtr, VarPtr> case_values;
        for (const SwitchCase& case_ : cases_)
            case_values.emplace(case_.value_expr(), case_.value_expr()->evaluate());

        Indexed<VarPtr> index_values;
        for (const auto& [index, case_expr] : indexed_case_exprs_)
            index_values.emplace(index, case_values.at(case_expr));

        // indices past the last input of a switch node select the last input, so a single switch node is enough if
        // every index from the last input onwards selects the same value
        unordered_set<VarPtr> last_input_values;
        for (const auto& [index, value] : index_values)
        {
            if (index >= SwitchInputCount - 1)
                last_input_values.insert(value);
        }

        if (last_input_values.size() <= 1)
        {
            Indexed<VarPtr> inputs;
            for (const auto& [index, value] : index_values)
                inputs[std::min(index, SwitchInputCount - 1)] = value;

            return all_equal(inputs) ? inputs.begin()->second : create_switch_node(inputs, which);
        }

        // otherwise, the outer switch node selects a group of 10 indices using which / 10, and the inner switch
        // nodes select an index in the group using which % 10, as floats since there is no integer modulo node
        vector<Indexed<VarPtr>> groups(SwitchInputCount);
        for (const auto& [index, value] : index_values)
            groups[index / SwitchInputCount].emplace(index % SwitchInputCount, value);

        const VarPtr float_which = which->type()->is<int>() ? runtime_utils::invoke_function(Type::Float, "convert", which) : which;
        const VarPtr outer_which = runtime_utils::invoke_function("divide", ArgumentList{float_which, static_cast<float>(SwitchInputCount)});
        VarPtr inner_which;

        Indexed<VarPtr> outer_inputs;
        for (size_t group = 0; group < groups.size(); ++group)
        {
            const Indexed<VarPtr>& inner_inputs = groups[group];
            if (inner_inputs.empty())
                continue;

            if (all_equal(inner_inputs))
            {
                outer_inputs.emplace(group, inner_inputs.begin()->second);
            }
            else
            {
                if (not inner_which)
                    inner_which = runtime_utils::invoke_function("modulo", ArgumentList{float_which, static_cast<float>(SwitchInputCount)});

                outer_inputs.emplace(group, create_switch_node(inner_inputs, inner_which));
            }
        }

        return create_switch_node(outer_inputs, outer_which);
    }

    string SwitchExpression::to_string() const
    {
        string result = "switch (" + which_expr_->to_string() + ")\n{";

        for (size_t i = 0; i < cases_.size(); ++i)
            result += (i == 0 ? "\n    " : ",\n    ") + cases_[i].to_string();

        return result + "\n}";
    }
}

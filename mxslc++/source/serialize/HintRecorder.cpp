//
// Created by jaket on 28/09/2026.
//

#include "serialize/HintRecorder.h"

#include "expressions/DotOperator.h"
#include "expressions/Identifier.h"
#include "expressions/IndexingOperator.h"
#include "expressions/UnnamedConstructor.h"
#include "expressions/interface.h"
#include "runtime/Argument.h"
#include "runtime/ArgumentList.h"
#include "runtime/Field.h"
#include "runtime/Function.h"
#include "runtime/Parameter.h"
#include "runtime/ParameterList.h"
#include "runtime/Type.h"
#include "runtime/variables/Variable.h"
#include "serialize/decompile_hints.h"
#include "serialize/values/interface.h"
#include "serialize/values/NodeOutputValue.h"
#include "serialize/values/NodeValue.h"
#include "statements/Statement.h"
#include "utils/string_utils.h"

namespace mxslc::serialize
{
    using string_utils::starts_with;

    namespace
    {
        void append_attribute(const mx::ElementPtr& element, const string& name, const string& value)
        {
            const string& current = element->getAttribute(name);
            if (current.empty())
            {
                element->setAttribute(name, value);
                return;
            }

            const vector<string> values = mx::splitString(current, " ");
            if (std::find(values.begin(), values.end(), value) == values.end())
                element->setAttribute(name, current + " " + value);
        }

        string argument_list(const FuncPtr& func, const ArgumentList& args)
        {
            string result;
            for (const Argument& arg : args)
            {
                const Parameter& param = func->parameters()[arg];
                result += " " + param.name();
                if (arg.has_name())
                    result += "=";
            }
            return result;
        }

        // the code of an argument without its name, e.g., "out fragColor"
        string argument_code(const Argument& arg)
        {
            const string mods = arg.modifiers().to_string();
            return (mods.empty() ? "" : mods + " ") + arg.expression()->to_string();
        }
    }

    void HintRecorder::finalise() const
    {
        if (not is_enabled_)
            return;

        // nodes renamed after compilation, e.g., in a node graph editor, are decompiled with their new name
        for (const mx::ElementPtr& element : doc_->traverseTree())
        {
            const mx::NodePtr node = element->asA<mx::Node>();
            if (node == nullptr)
                continue;

            const vector<string> bindings = mx::splitString(node->getAttribute(hints::BIND_ATTRIBUTE), " ");
            const bool is_variable = std::any_of(bindings.begin(), bindings.end(), [](const string& binding) { return starts_with(binding, "s"); });
            if (is_variable)
                node->setAttribute(hints::NAME_ATTRIBUTE, node->getName());
        }
    }

    void HintRecorder::write_function(const mx::ElementPtr& element) const
    {
        if (not is_enabled_ or saved_frames_.empty())
            return;

        // the function is written while its body is recorded, so the definition is part of the saved frames
        const vector<Frame>& frames = saved_frames_.back();
        for (auto it = frames.rbegin(); it != frames.rend(); ++it)
        {
            if (it->kind == FrameKind::Statement)
            {
                element->setAttribute(hints::DEFINITION_ATTRIBUTE, "s" + std::to_string(it->id));
                return;
            }
        }
    }

    HintRecorder::Snapshot HintRecorder::snapshot() const
    {
        if (not is_enabled_)
            return nullptr;
        return std::make_shared<const vector<Frame>>(frames_);
    }

    HintRecorder::RestoredSnapshot::RestoredSnapshot(HintRecorder& hints, const Snapshot& snapshot) : hints_{hints}
    {
        if (not hints_.is_enabled_ or snapshot == nullptr)
            return;

        hints_.saved_frames_.push_back(std::move(hints_.frames_));
        hints_.frames_ = *snapshot;
        is_restored_ = true;
    }

    HintRecorder::RestoredSnapshot::~RestoredSnapshot()
    {
        if (not is_restored_)
            return;

        hints_.frames_ = std::move(hints_.saved_frames_.back());
        hints_.saved_frames_.pop_back();
    }

    HintRecorder::HiddenFrame::HiddenFrame(HintRecorder& hints) : hints_{hints}
    {
        if (not hints_.is_enabled_)
            return;
        hints_.frames_.push_back(Frame{FrameKind::Hidden});
        is_pushed_ = true;
    }

    HintRecorder::HiddenFrame::~HiddenFrame()
    {
        if (is_pushed_ and not hints_.frames_.empty())
            hints_.frames_.pop_back();
    }

    void HintRecorder::enter_statement(const statements::Statement& stmt)
    {
        if (not is_enabled_)
            return;

        if (is_hidden())
        {
            frames_.push_back(Frame{FrameKind::Hidden});
            return;
        }

        if (not stmt.is_hinted())
        {
            frames_.push_back(Frame{FrameKind::Transparent});
            return;
        }

        // the copies of a statement for the template types of a function are the same statement
        const auto [it, is_new] = statement_ids_.try_emplace(&stmt.origin(), next_statement_id_);
        if (is_new)
        {
            ++next_statement_id_;
            statement_parents_[it->second] = parent_id();
        }

        frames_.push_back(Frame{FrameKind::Statement, it->second});
    }

    void HintRecorder::exit_statement(const statements::Statement& stmt)
    {
        if (not is_enabled_ or frames_.empty())
            return;

        const Frame frame = frames_.back();
        frames_.pop_back();

        if (frame.kind != FrameKind::Statement or recorded_statements_.count(frame.id) > 0)
            return;
        recorded_statements_.insert(frame.id);

        // the skeleton can contain values that are only known after the statement has been executed
        try
        {
            const string skeleton = stmt.hint_skeleton();
            if (not skeleton.empty())
                doc_->setAttribute(hints::STATEMENT_PREFIX + std::to_string(frame.id), std::to_string(statement_parents_[frame.id]) + "|" + hints::encode(skeleton));
        }
        catch (...)
        {
            // hints are best-effort, a statement without a record is decompiled without hints
        }
    }

    void HintRecorder::enter_loop_iteration(const size_t index)
    {
        if (not is_enabled_)
            return;
        frames_.push_back(is_hidden() ? Frame{FrameKind::Hidden} : Frame{FrameKind::Loop, index});
    }

    void HintRecorder::exit_loop_iteration()
    {
        if (is_enabled_ and not frames_.empty())
            frames_.pop_back();
    }

    void HintRecorder::enter_call(const FuncPtr& func, const ArgumentList& args, const vector<VarPtr>& arg_values)
    {
        if (not is_enabled_)
            return;

        if (is_hidden())
        {
            frames_.push_back(Frame{FrameKind::Hidden});
            return;
        }

        Frame frame{FrameKind::Call, next_call_id_++};
        if (const auto it = function_ids_.find(func.get()); it != function_ids_.end())
            frame.definition_id = it->second;

        frame.function = func->name();
        if (frame.definition_id > 0)
            frame.function += "@s" + std::to_string(frame.definition_id);

        // compile-time values and out arguments cannot be recovered from the graph, so their code is recorded
        for (const Argument& arg : args)
        {
            const Parameter& param = func->parameters()[arg];
            const VarPtr& value = param.index() < arg_values.size() ? arg_values[param.index()] : nullptr;
            const bool is_compile_time = value and value->is_compile_time();
            const string code = param.is_out() or is_compile_time ? argument_code(arg) : "";
            frame.arguments.push_back(ArgumentRecord{param.name(), arg.has_name(), param.is_out(), code});
        }

        frames_.push_back(std::move(frame));
    }

    void HintRecorder::exit_call()
    {
        if (not is_enabled_ or frames_.empty())
            return;

        // the record is written again, because only now it is known which compile-time arguments were used by inputs
        const Frame& frame = frames_.back();
        if (frame.kind == FrameKind::Call and frame.is_recorded)
            doc_->setAttribute(hints::CALL_PREFIX + std::to_string(frame.id), hints::encode(call_record(frame)));
        frames_.pop_back();
    }

    string HintRecorder::call_record(const Frame& frame) const
    {
        // e.g., "mainImage@s4 fragColor:out%20fragColor fragCoord"
        string result = frame.function;
        for (const ArgumentRecord& arg : frame.arguments)
        {
            result += " " + arg.param + (arg.is_named ? "=" : "");
            const bool is_used = used_parameters_.count("c" + std::to_string(frame.id) + "." + arg.param) > 0;
            if (not arg.code.empty() and (arg.is_out or not is_used))
                result += ":" + hints::encode_argument(arg.code);
        }
        return result;
    }

    void HintRecorder::enter_function_body()
    {
        if (not is_enabled_)
            return;

        const Frame body = is_hidden() ? Frame{FrameKind::Hidden} : Frame{FrameKind::Function, 0, statement_id().value_or(0)};
        saved_frames_.push_back(std::move(frames_));
        frames_ = {body};
    }

    void HintRecorder::exit_function_body()
    {
        if (not is_enabled_ or saved_frames_.empty())
            return;

        frames_ = std::move(saved_frames_.back());
        saved_frames_.pop_back();
    }

    void HintRecorder::define_function(const FuncPtr& func)
    {
        if (not is_enabled_ or is_hidden())
            return;

        if (const optional<size_t> id = statement_id())
            function_ids_[func.get()] = *id;
    }

    void HintRecorder::define_variable(const VarPtr& var, const string& name)
    {
        if (not is_enabled_ or is_hidden())
            return;

        if (const optional<size_t> id = statement_id())
            owners_[var.get()] = Owner{var, "s" + std::to_string(*id) + "." + name};
    }

    void HintRecorder::define_parameter(const VarPtr& var, const string& name, const VarPtr& arg_value)
    {
        if (not is_enabled_ or frames_.empty())
            return;

        // the parameters of hidden calls are not recorded, but the values of their arguments are passed through them, e.g.,
        // to the separate node of the swizzle `e.yx`
        if (frames_.back().kind == FrameKind::Hidden)
        {
            if (arg_value)
                sources_[var.get()] = arg_value;
            return;
        }

        if (frames_.back().kind != FrameKind::Call)
            return;

        owners_[var.get()] = Owner{var, "c" + std::to_string(frames_.back().id) + "." + name, /*is_parameter*/true};
        if (arg_value)
            sources_[var.get()] = arg_value;
    }

    void HintRecorder::copy_variable(const VarPtr& var, const VarPtr& source)
    {
        if (not is_enabled_ or is_hidden() or var == nullptr or source == nullptr or var == source)
            return;
        sources_[var.get()] = source;
    }

    void HintRecorder::write_node(const mx::NodePtr& node, const ArgumentList& args, const FuncPtr& func)
    {
        if (not is_enabled_)
            return;

        write_call_records();

        const string context = path();
        if (not context.empty())
            node->setAttribute(hints::CONTEXT_ATTRIBUTE, context);

        const bool has_named_argument = std::any_of(args.begin(), args.end(), [](const Argument& arg) { return arg.has_name(); });
        if (has_named_argument)
            node->setAttribute(hints::ARGUMENTS_ATTRIBUTE, argument_list(func, args).substr(1));
    }

    void HintRecorder::write_input(const mx::NodePtr& node, const string& input_name, const VarPtr& value)
    {
        if (not is_enabled_ or value == nullptr)
            return;

        // follow the value through the parameters it was passed to
        string source;
        VarPtr var = value;
        unordered_set<const Variable*> visited;
        while (var and visited.insert(var.get()).second)
        {
            const auto owner = owners_.find(var.get());
            if (owner == owners_.end())
            {
                // the parameters of hidden calls pass their values through
                if (const auto hidden_source = sources_.find(var.get()); hidden_source != sources_.end())
                {
                    var = hidden_source->second;
                    continue;
                }

                // or the field of a struct variable, e.g., "s3.xyz.1"
                if (const optional<string> field = field_source(var))
                {
                    source += (source.empty() ? "" : " ") + *field;
                    if (const mx::InputPtr input = node->getInput(input_name); input and input->hasValue() and var->has_value() and var->is_compile_time())
                        input->setAttribute(hints::VALUE_ATTRIBUTE, input->getValueString());
                }
                break;
            }

            if (not owner->second.is_parameter)
            {
                // named variables are only recorded if their value is not a node, i.e., it cannot be named by the node,
                // or if they are a copy of another variable, e.g., `vec2 uv = fragCoord;` where fragCoord is a parameter
                const bool is_compile_time = var->has_value() and var->is_compile_time();
                const auto copy_source = sources_.find(var.get());
                if (is_compile_time or copy_source != sources_.end())
                    source += (source.empty() ? "" : " ") + owner->second.name;
                if (is_compile_time)
                {
                    // the variable is only referenced if the value is not edited
                    if (const mx::InputPtr input = node->getInput(input_name); input and input->hasValue())
                        input->setAttribute(hints::VALUE_ATTRIBUTE, input->getValueString());
                    break;
                }
                if (copy_source == sources_.end())
                    break;
                var = copy_source->second;
                continue;
            }

            source += (source.empty() ? "" : " ") + owner->second.name;
            used_parameters_.insert(owner->second.name);
            const auto arg_value = sources_.find(var.get());
            var = arg_value != sources_.end() ? arg_value->second : nullptr;
        }

        if (source.empty())
            return;

        if (const mx::InputPtr input = node->getInput(input_name))
            input->setAttribute(hints::SOURCE_ATTRIBUTE, source);
    }

    optional<string> HintRecorder::field_source(const VarPtr& var) const
    {
        // the path from the struct variable to the field, e.g., ".xyz.1" or ".ray.origin"
        string path;
        VarPtr field = var;
        while (field->has_parent())
        {
            const VarPtr parent = field->parent();
            size_t index = 0;
            while (index < parent->child_count() and parent->child(index) != field)
                ++index;
            if (index == parent->child_count())
                return std::nullopt;

            const Field& type_field = parent->type()->field(index);
            path = "." + (type_field.has_name() ? type_field.name() : std::to_string(index)) + path;
            field = parent;

            // fields of parameters are not recorded, their values are passed through the parameters
            if (const auto owner = owners_.find(field.get()); owner != owners_.end())
                return owner->second.is_parameter ? std::nullopt : optional<string>{owner->second.name + path};
        }
        return std::nullopt;
    }

    void HintRecorder::bind_variable(const VarPtr& var, const string& name)
    {
        if (not is_enabled_ or is_hidden() or var == nullptr)
            return;

        const optional<size_t> id = statement_id();
        if (not id)
            return;

        const string target = "s" + std::to_string(*id) + "." + name;
        bind(var, target, /*is_statement*/true);

        // the variable holds the value assigned by the statement, e.g., a compile-time value that is used by an input
        if (owners_.count(var.get()) > 0)
        {
            owners_[var.get()] = Owner{var, target};
            sources_.erase(var.get());
        }
    }

    void HintRecorder::bind_return_value(const VarPtr& value)
    {
        if (not is_enabled_ or frames_.empty() or frames_.back().kind != FrameKind::Call or value == nullptr)
            return;

        // compile-time values are not part of the graph, so the inputs they are used by name the call, e.g., "c4"
        if (value->is_compile_time())
        {
            owners_[value.get()] = Owner{value, "c" + std::to_string(frames_.back().id)};
            return;
        }

        bind(value, "c" + std::to_string(frames_.back().id));
    }

    void HintRecorder::bind_out_parameter(const VarPtr& value, const string& param_name)
    {
        if (not is_enabled_ or frames_.empty() or frames_.back().kind != FrameKind::Call or value == nullptr)
            return;

        bind(value, "c" + std::to_string(frames_.back().id) + "." + param_name);
    }

    void HintRecorder::bind_input(const mx::InputPtr& input)
    {
        if (not is_enabled_ or is_hidden())
            return;

        if (const optional<size_t> id = statement_id())
            input->setAttribute(hints::BIND_ATTRIBUTE, "s" + std::to_string(*id));
    }

    string HintRecorder::value_skeleton(const VarPtr& var)
    {
        if (var == nullptr)
            return hints::PLACEHOLDER;

        if (var->has_value())
            return var->is_compile_time() ? var->compile_time_value().to_string() : hints::PLACEHOLDER;

        string result;
        bool has_compile_time_field = false;
        for (size_t i = 0; i < var->child_count(); ++i)
        {
            const string field = value_skeleton(var->child(i));
            has_compile_time_field = has_compile_time_field or field != hints::PLACEHOLDER;
            result += (i > 0 ? ", " : "") + field;
        }
        return has_compile_time_field ? "{" + result + "}" : hints::PLACEHOLDER;
    }

    string HintRecorder::expression_skeleton(const ExprPtr& expr, const VarPtr& value)
    {
        if (expr == nullptr or value == nullptr)
            return hints::PLACEHOLDER;

        // compile-time values are not part of the graph, so their code is recorded, e.g., `foo()`
        if (value->is_compile_time())
            return expr->to_string();

        // or the code of the fields that are compile-time values, e.g., `{_, 1.0}`
        const auto constructor = cast_expression<UnnamedConstructor>(expr);
        if (constructor and not value->has_value() and constructor->expressions().size() == value->child_count())
        {
            string result;
            bool has_compile_time_field = false;
            for (size_t i = 0; i < value->child_count(); ++i)
            {
                const string field = expression_skeleton(constructor->expressions()[i], value->child(i));
                has_compile_time_field = has_compile_time_field or field != hints::PLACEHOLDER;
                result += (i > 0 ? ", " : "") + field;
            }
            return has_compile_time_field ? "{" + result + "}" : hints::PLACEHOLDER;
        }

        return value_skeleton(value);
    }

    VarPtr HintRecorder::assigned_variable(const ExprPtr& lvalue)
    {
        if (const auto identifier = cast_expression<Identifier>(lvalue))
            return identifier->variable();
        if (const auto dot = cast_expression<DotOperator>(lvalue))
            return assigned_variable(dot->value_expression());
        if (const auto indexing = cast_expression<IndexingOperator>(lvalue))
            return assigned_variable(indexing->value_expression());
        return nullptr;
    }

    bool HintRecorder::is_hidden() const
    {
        if (frames_.empty())
            return false;

        // nothing inside of library functions is recorded
        const Frame& frame = frames_.back();
        return frame.kind == FrameKind::Hidden or (frame.kind == FrameKind::Call and frame.definition_id == 0);
    }

    optional<size_t> HintRecorder::statement_id() const
    {
        for (auto it = frames_.rbegin(); it != frames_.rend(); ++it)
        {
            if (it->kind == FrameKind::Statement)
                return it->id;
        }
        return std::nullopt;
    }

    size_t HintRecorder::parent_id() const
    {
        for (auto it = frames_.rbegin(); it != frames_.rend(); ++it)
        {
            if (it->kind == FrameKind::Statement)
                return it->id;
            // statements in the body of a function are part of the function definition
            if (it->kind == FrameKind::Call or it->kind == FrameKind::Function)
                return it->definition_id;
        }
        return 0;
    }

    string HintRecorder::path(const size_t frame_count) const
    {
        string result;
        for (size_t i = 0; i < std::min(frame_count, frames_.size()); ++i)
        {
            const Frame& frame = frames_[i];
            string name;
            if (frame.kind == FrameKind::Statement)
                name = "s" + std::to_string(frame.id);
            else if (frame.kind == FrameKind::Loop)
                name = "l" + std::to_string(frame.id);
            else if (frame.kind == FrameKind::Call)
                name = "c" + std::to_string(frame.id);
            else
                continue;

            result += (result.empty() ? "" : " ") + name;
        }
        return result;
    }

    bool HintRecorder::is_created_in_current_execution(const mx::NodePtr& node) const
    {
        // the frames up to the innermost call or loop iteration, any node of the graph if there are none
        size_t frame_count = 0;
        for (size_t i = 0; i < frames_.size(); ++i)
        {
            if (frames_[i].kind == FrameKind::Call or frames_[i].kind == FrameKind::Loop)
                frame_count = i + 1;
        }
        if (frame_count == 0)
            return true;

        const string execution = path(frame_count);
        const string& context = node->getAttribute(hints::CONTEXT_ATTRIBUTE);
        return context == execution or starts_with(context, execution + " ");
    }

    bool HintRecorder::is_created_in_current_frame(const mx::NodePtr& node) const
    {
        const string current = path();
        if (current.empty())
            return false;

        const string& context = node->getAttribute(hints::CONTEXT_ATTRIBUTE);
        return context == current or starts_with(context, current + " ");
    }

    void HintRecorder::bind(const VarPtr& var, const string& target, const bool is_statement)
    {
        if (var->has_value())
        {
            const ValuePtr value = var->raw_value();

            mx::NodePtr node;
            string output_name;
            if (const NodeValuePtr node_value = cast_value<NodeValue>(value))
            {
                node = node_value->node();
            }
            else if (const NodeOutputValuePtr output_value = cast_value<NodeOutputValue>(value))
            {
                node = output_value->node();
                output_name = output_value->output_name();
            }

            // values that were created before the current frame are already bound to where they were created, but statements
            // also bind the values they assign that were created earlier by the same execution of the statement, i.e., in the
            // same call or loop iteration, e.g., `float y = x;`
            if (node == nullptr)
                return;
            if (not is_created_in_current_frame(node) and not (is_statement and is_created_in_current_execution(node)))
                return;

            append_attribute(node, hints::BIND_ATTRIBUTE, output_name.empty() ? target : target + ":" + output_name);
        }
        else
        {
            const TypePtr& type = var->type();
            for (size_t i = 0; i < var->child_count(); ++i)
            {
                const Field& field = type->field(i);
                bind(var->child(i), target + "." + (field.has_name() ? field.name() : std::to_string(i)), is_statement);
            }
        }
    }

    void HintRecorder::write_call_records()
    {
        for (Frame& frame : frames_)
        {
            if (frame.kind != FrameKind::Call or frame.is_recorded)
                continue;
            doc_->setAttribute(hints::CALL_PREFIX + std::to_string(frame.id), hints::encode(call_record(frame)));
            frame.is_recorded = true;
        }
    }

    StatementHintFrame::StatementHintFrame(HintRecorder& hints, const statements::Statement& stmt)
        : hints_{hints}, stmt_{stmt}
    {
        hints_.enter_statement(stmt_);
    }

    StatementHintFrame::~StatementHintFrame()
    {
        hints_.exit_statement(stmt_);
    }
}

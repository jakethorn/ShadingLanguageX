//
// Created by jaket on 28/09/2026.
//

#include "decompile/DocumentHints.h"

#include "parse.h"
#include "scan.h"
#include "serialize/decompile_hints.h"
#include "statements/FunctionDefinition.h"
#include "statements/Statement.h"
#include "utils/string_utils.h"

namespace mxslc::decompile
{
    using string_utils::starts_with;
    namespace hints = serialize::hints;

    namespace
    {
        optional<size_t> parse_id(const string& str)
        {
            if (str.empty() or not std::all_of(str.begin(), str.end(), [](const char c) { return std::isdigit(static_cast<unsigned char>(c)); }))
                return std::nullopt;
            return static_cast<size_t>(std::stoull(str));
        }

        optional<DocumentHints::Frame> parse_frame(const string& str)
        {
            if (str.size() < 2 or string{"slc"}.find(str.front()) == string::npos)
                return std::nullopt;
            const optional<size_t> id = parse_id(str.substr(1));
            if (not id)
                return std::nullopt;
            return DocumentHints::Frame{str.front(), *id};
        }

        vector<string> split(const string& str, const char delimiter)
        {
            vector<string> result;
            size_t start = 0;
            while (start <= str.size())
            {
                size_t end = str.find(delimiter, start);
                if (end == string::npos)
                    end = str.size();
                if (end > start)
                    result.push_back(str.substr(start, end - start));
                start = end + 1;
            }
            return result;
        }

        shared_ptr<statements::Statement> parse_skeleton(const string& skeleton)
        {
            try
            {
                Parser parser{DocumentHints::scan_skeleton(skeleton)};
                return parser.statement();
            }
            catch (...)
            {
                return nullptr;
            }
        }

        bool has_placeholder(const string& skeleton)
        {
            try
            {
                for (const Token& token : scan_string(skeleton))
                {
                    if (token == TokenType::Identifier and token.lexeme() == hints::PLACEHOLDER)
                        return true;
                }
                return false;
            }
            catch (...)
            {
                return true;
            }
        }

        vector<DocumentHints::Argument> parse_arguments(const vector<string>& words)
        {
            vector<DocumentHints::Argument> result;
            for (const string& word : words)
            {
                // e.g., "t", "low=" or "fragColor:out%20fragColor"
                const size_t colon = word.find(':');
                const string param = word.substr(0, colon);
                optional<string> code;
                if (colon != string::npos)
                    code = hints::decode_argument(word.substr(colon + 1));

                const bool is_named = not param.empty() and param.back() == '=';
                result.push_back(DocumentHints::Argument{is_named ? param.substr(0, param.size() - 1) : param, is_named, std::move(code)});
            }
            return result;
        }
    }

    DocumentHints::DocumentHints(const mx::DocumentPtr& doc)
    {
        for (const string& name : doc->getAttributeNames())
        {
            const string value = hints::decode(doc->getAttribute(name));

            if (starts_with(name, hints::STATEMENT_PREFIX))
            {
                const optional<size_t> id = parse_id(name.substr(hints::STATEMENT_PREFIX.size()));
                const size_t separator = value.find('|');
                if (not id or separator == string::npos)
                    continue;
                const optional<size_t> parent = parse_id(value.substr(0, separator));
                if (not parent)
                    continue;

                const string skeleton = value.substr(separator + 1);
                statements_[*id] = StatementRecord{*id, *parent, skeleton, parse_skeleton(skeleton), not has_placeholder(skeleton)};
            }
            else if (starts_with(name, hints::CALL_PREFIX))
            {
                const optional<size_t> id = parse_id(name.substr(hints::CALL_PREFIX.size()));
                const vector<string> words = split(value, ' ');
                if (not id or words.empty())
                    continue;

                // e.g., "sq@s1 v" or "__mul__ in1 in2"
                string function = words.front();
                size_t definition = 0;
                if (const size_t at = function.find("@s"); at != string::npos)
                {
                    const optional<size_t> definition_id = parse_id(function.substr(at + 2));
                    if (not definition_id)
                        continue;
                    definition = *definition_id;
                    function = function.substr(0, at);
                }

                calls_[*id] = CallRecord{*id, function, definition, parse_arguments({words.begin() + 1, words.end()})};
            }
        }
    }

    const DocumentHints::StatementRecord* DocumentHints::statement(const size_t id) const
    {
        const auto it = statements_.find(id);
        return it != statements_.end() ? &it->second : nullptr;
    }

    const DocumentHints::CallRecord* DocumentHints::call(const size_t id) const
    {
        const auto it = calls_.find(id);
        return it != calls_.end() ? &it->second : nullptr;
    }

    vector<size_t> DocumentHints::children(const size_t parent) const
    {
        vector<size_t> result;
        for (const auto& [id, record] : statements_)
        {
            if (record.parent == parent)
                result.push_back(id);
        }
        return result;
    }

    bool DocumentHints::is_inline_function(const size_t definition_id) const
    {
        const StatementRecord* record = statement(definition_id);
        const auto func_def = record ? std::dynamic_pointer_cast<statements::FunctionDefinition>(record->statement) : nullptr;
        if (func_def == nullptr or not func_def->is_inline())
            return false;

        try
        {
            const vector<Token> tokens = scan_skeleton(record->skeleton);
            return std::none_of(tokens.begin(), tokens.end(), [](const Token& token) { return token == TokenType::Comptime; });
        }
        catch (...)
        {
            return false;
        }
    }

    vector<DocumentHints::Frame> DocumentHints::context(const mx::ElementPtr& element)
    {
        vector<Frame> result;
        for (const string& word : split(element->getAttribute(hints::CONTEXT_ATTRIBUTE), ' '))
        {
            const optional<Frame> frame = parse_frame(word);
            if (not frame)
                return {};
            result.push_back(*frame);
        }
        return result;
    }

    vector<DocumentHints::Binding> DocumentHints::bindings(const mx::ElementPtr& element)
    {
        vector<Binding> result;
        for (const string& word : split(element->getAttribute(hints::BIND_ATTRIBUTE), ' '))
        {
            // e.g., "s9.ray.origin" or "s9.u:outx"
            const size_t colon = word.find(':');
            const string target = word.substr(0, colon);
            const string output = colon != string::npos ? word.substr(colon + 1) : "";

            vector<string> parts = split(target, '.');
            if (parts.empty())
                continue;
            const optional<Frame> frame = parse_frame(parts.front());
            if (not frame)
                continue;

            parts.erase(parts.begin());
            // statement bindings are always to a variable
            if (frame->kind == 's' and parts.empty())
                continue;
            result.push_back(Binding{*frame, std::move(parts), output});
        }
        return result;
    }

    vector<DocumentHints::Source> DocumentHints::sources(const mx::ElementPtr& element)
    {
        vector<Source> result;
        for (const string& word : split(element->getAttribute(hints::SOURCE_ATTRIBUTE), ' '))
        {
            // a call without a variable is the compile-time return value of the call, e.g., "c4"
            const size_t dot = word.find('.');
            const optional<Frame> frame = parse_frame(word.substr(0, dot));
            if (not frame or (dot == string::npos and frame->kind != 'c'))
                return {};
            result.push_back(Source{*frame, dot == string::npos ? "" : word.substr(dot + 1)});
        }
        return result;
    }

    optional<vector<DocumentHints::Argument>> DocumentHints::arguments(const mx::ElementPtr& element)
    {
        if (not element->hasAttribute(hints::ARGUMENTS_ATTRIBUTE))
            return std::nullopt;
        return parse_arguments(split(element->getAttribute(hints::ARGUMENTS_ATTRIBUTE), ' '));
    }

    optional<size_t> DocumentHints::definition(const mx::ElementPtr& element)
    {
        const optional<Frame> frame = parse_frame(element->getAttribute(hints::DEFINITION_ATTRIBUTE));
        if (frame and frame->kind == 's')
            return frame->id;
        return std::nullopt;
    }

    optional<size_t> DocumentHints::assignment(const mx::ElementPtr& input)
    {
        const optional<Frame> frame = parse_frame(input->getAttribute(hints::BIND_ATTRIBUTE));
        if (frame and frame->kind == 's')
            return frame->id;
        return std::nullopt;
    }

    vector<Token> DocumentHints::scan_skeleton(const string& skeleton)
    {
        vector<Token> tokens = scan_string(skeleton);
        tokens.erase(std::remove(tokens.begin(), tokens.end(), TokenType::Newline), tokens.end());
        return tokens;
    }

    bool DocumentHints::is_unedited(const mx::InputPtr& input)
    {
        if (not input->hasAttribute(hints::VALUE_ATTRIBUTE) or not input->hasValue())
            return false;
        if (not input->getNodeName().empty() or not input->getNodeGraphString().empty() or not input->getInterfaceName().empty())
            return false;

        // values are compared by value, e.g., "0,0" is the same value as "0, 0"
        const mx::ValuePtr value = input->getValue();
        const mx::ValuePtr compiled_value = mx::Value::createValueFromStrings(input->getAttribute(hints::VALUE_ATTRIBUTE), input->getType());
        return value and compiled_value and value->getValueString() == compiled_value->getValueString();
    }

    bool DocumentHints::is_hint(const string& attribute_name)
    {
        return starts_with(attribute_name, hints::PREFIX);
    }
}

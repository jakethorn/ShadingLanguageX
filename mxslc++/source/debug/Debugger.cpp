//
// Created by jaket on 10/08/2026.
//

#include <iostream>
#include <sstream>
#include <string>
#include <cctype>

#include "debug/Debugger.h"
#include "errors/CompileError.h"
#include "runtime/Scope.h"
#include "runtime/Runtime.h"
#include "runtime/Type.h"
#include "runtime/variables/Variable.h"
#include "statements/Statement.h"

namespace mxslc::debug
{
    namespace
    {
        constexpr string_view blue = "\033[94m";
        constexpr string_view red = "\033[31m";
        constexpr string_view green = "\033[32m";
        constexpr string_view yellow = "\033[33m";
        constexpr string_view orange = "\033[38;5;208m";
        constexpr string_view magenta = "\033[95m";
        constexpr string_view reset = "\033[0m";

        const unordered_set<string> data_types{
            "BSDF", "EDF", "T", "VDF", "auto", "bool", "boolean", "color3", "color4",
            "displacementshader", "filename", "float", "int", "integer", "lightshader", "mat3",
            "mat4", "material", "matrix33", "matrix44", "string", "surfaceshader", "vec2", "vec3",
            "vec4", "vector2", "vector3", "vector4", "void", "volumeshader"
        };

        bool is_identifier_character(const char c)
        {
            return std::isalnum(static_cast<unsigned char>(c)) or c == '_';
        }

        string highlight_keywords(const string& code)
        {
            string result;
            bool in_block_comment = false;
            bool in_string = false;

            for (size_t i = 0; i < code.size();)
            {
                if (in_block_comment)
                {
                    result += code[i];
                    if (code[i] == '*' and i + 1 < code.size() and code[i + 1] == '/')
                    {
                        result += code[++i];
                        result += string{reset};
                        in_block_comment = false;
                    }
                    ++i;
                    continue;
                }

                if (in_string)
                {
                    result += code[i];
                    if (code[i] == '\\' and i + 1 < code.size())
                        result += code[++i];
                    else if (code[i] == '"')
                    {
                        result += string{reset};
                        in_string = false;
                    }
                    ++i;
                    continue;
                }

                if (code[i] == '/' and i + 1 < code.size() and code[i + 1] == '/')
                {
                    result += string{green};
                    result.append(code, i, code.find('\n', i) - i);
                    result += string{reset};
                    i = code.find('\n', i);
                    if (i == string::npos)
                        break;
                    continue;
                }
                if (code[i] == '/' and i + 1 < code.size() and code[i + 1] == '*')
                {
                    result += string{green} + "/*";
                    i += 2;
                    in_block_comment = true;
                    continue;
                }
                if (code[i] == '"')
                {
                    result += string{green} + code[i++];
                    in_string = true;
                    continue;
                }

                if (code[i] == '#' and i + 1 < code.size()
                    and (std::isalpha(static_cast<unsigned char>(code[i + 1])) or code[i + 1] == '_'))
                {
                    const size_t begin = i++;
                    while (i < code.size()
                           and (std::isalnum(static_cast<unsigned char>(code[i])) or code[i] == '_'))
                        ++i;
                    result += string{yellow} + code.substr(begin, i - begin) + string{reset};
                    continue;
                }

                if (std::isdigit(static_cast<unsigned char>(code[i]))
                    or (code[i] == '.' and i + 1 < code.size()
                        and std::isdigit(static_cast<unsigned char>(code[i + 1]))))
                {
                    const size_t begin = i;
                    while (i < code.size() and std::isdigit(static_cast<unsigned char>(code[i])))
                        ++i;
                    if (i < code.size() and code[i] == '.')
                    {
                        ++i;
                        while (i < code.size() and std::isdigit(static_cast<unsigned char>(code[i])))
                            ++i;
                    }
                    if (i < code.size() and (code[i] == 'e' or code[i] == 'E'))
                    {
                        ++i;
                        if (i < code.size() and (code[i] == '+' or code[i] == '-'))
                            ++i;
                        while (i < code.size() and std::isdigit(static_cast<unsigned char>(code[i])))
                            ++i;
                    }
                    if (i < code.size() and (code[i] == 'f' or code[i] == 'F'))
                        ++i;
                    result += string{blue} + code.substr(begin, i - begin) + string{reset};
                    continue;
                }

                if (std::isalpha(static_cast<unsigned char>(code[i])) or code[i] == '_')
                {
                    const size_t begin = i++;
                    while (i < code.size() and is_identifier_character(code[i]))
                        ++i;
                    const string word = code.substr(begin, i - begin);
                    const TokenType type{word};
                    if (word == "true" or word == "false")
                        result += string{blue} + word + string{reset};
                    else if (data_types.find(word) != data_types.end())
                        result += string{orange} + word + string{reset};
                    else if (TokenType::Keywords.find(static_cast<TokenType::Enum>(type.index())) != TokenType::Keywords.end())
                        result += string{magenta} + word + string{reset};
                    else
                        result += word;
                    continue;
                }

                result += code[i++];
            }
            return result;
        }

        string highlight_xml(const string& xml)
        {
            string result;
            bool in_tag = false;
            bool in_tag_name = false;
            bool in_string = false;

            for (size_t i = 0; i < xml.size(); ++i)
            {
                const char c = xml[i];

                if (in_string)
                {
                    result += c;
                    if (c == '"')
                    {
                        result += string{reset};
                        in_string = false;
                    }
                    continue;
                }

                if (in_tag and c == '"')
                {
                    result += string{green} + c;
                    in_string = true;
                    continue;
                }

                if (not in_tag and c == '<')
                {
                    result += string{yellow} + c;
                    in_tag = true;
                    in_tag_name = true;
                    continue;
                }

                if (in_tag and c == '>')
                {
                    result += string{yellow} + c + string{reset};
                    in_tag = false;
                    in_tag_name = false;
                    continue;
                }

                if (in_tag and (c == '/' or c == '?') and i + 1 < xml.size() and xml[i + 1] == '>')
                {
                    result += string{yellow} + c + string{reset};
                    continue;
                }

                if (in_tag and c == '=')
                {
                    result += string{green} + c + string{reset};
                    continue;
                }

                if (in_tag and in_tag_name and std::isspace(static_cast<unsigned char>(c)))
                {
                    result += string{reset};
                    in_tag_name = false;
                }

                if (in_tag and in_tag_name)
                    result += string{yellow} + c;
                else
                    result += c;
            }

            if (in_string or in_tag)
                result += string{reset};
            return result;
        }

        vector<string> lines(const string& text)
        {
            vector<string> result;
            std::istringstream stream{text};
            string line;
            while (std::getline(stream, line))
                result.push_back(std::move(line));
            if (result.empty())
                result.emplace_back();
            return result;
        }

        vector<bool> changed_lines(const vector<string>& previous, const vector<string>& current)
        {
            const size_t previous_count = previous.size();
            const size_t current_count = current.size();
            vector<vector<size_t>> lcs(current_count + 1, vector<size_t>(previous_count + 1));

            for (size_t current_index = current_count; current_index-- > 0;)
            {
                for (size_t previous_index = previous_count; previous_index-- > 0;)
                {
                    if (current[current_index] == previous[previous_index])
                        lcs[current_index][previous_index] = lcs[current_index + 1][previous_index + 1] + 1;
                    else
                        lcs[current_index][previous_index] = std::max(
                            lcs[current_index + 1][previous_index], lcs[current_index][previous_index + 1]);
                }
            }

            vector<bool> changed(current_count, true);
            size_t current_index = 0;
            size_t previous_index = 0;
            while (current_index < current_count and previous_index < previous_count)
            {
                if (current[current_index] == previous[previous_index])
                {
                    changed[current_index] = false;
                    ++current_index;
                    ++previous_index;
                }
                else if (lcs[current_index + 1][previous_index] >= lcs[current_index][previous_index + 1])
                    ++current_index;
                else
                    ++previous_index;
            }
            return changed;
        }
    }

    unique_ptr<Debugger> Debugger::instance_ = nullptr;

    Debugger::Debugger(string code, optional<fs::path> src_path) : code_{std::move(code)}, src_path_{std::move(src_path)}
    {

    }

    Debugger& Debugger::create(string code, optional<fs::path> src_path)
    {
        instance_ = std::unique_ptr<Debugger>{new Debugger{std::move(code), std::move(src_path)}};
        return *instance_;
    }

    Debugger& Debugger::get()
    {
        if (instance_ == nullptr)
            throw CompileError{"Debugger not created"};
        return *instance_;
    }

    bool Debugger::is_enabled()
    {
        return instance_ != nullptr;
    }

    void Debugger::close()
    {
        if (is_enabled())
            get().next_statement(nullptr);
    }

    void Debugger::next_statement(const Statement* stmt)
    {
        if (stmt)
        {
            if (src_path_)
            {
                if (src_path_->filename().string() != stmt->token().filename())
                    return;
            }
            else
            {
                if (not stmt->token().filename().empty())
                    return;
            }
        }

        vector<string> scope_lines;
        for (const VarPtr& variable : runtime::Runtime::get().scope().get_all_variables())
        {
            const string variable_text = variable->type()->to_string() + " " + variable->name()
                                        + " = " + variable->to_string();
            const vector<string> variable_lines = lines(variable_text);
            scope_lines.insert(scope_lines.end(), variable_lines.begin(), variable_lines.end());
        }

        const vector<string> code_lines = lines(code_);
        const vector<string> highlighted_code_lines = lines(highlight_keywords(code_));
        const string current_xml = runtime::Runtime::get().serializer().xml();
        const vector<string> current_xml_lines = lines(current_xml);
        const vector<bool> xml_changed = changed_lines(previous_xml_lines_, current_xml_lines);
        const vector<string> xml_lines = lines(highlight_xml(current_xml));
        previous_xml_lines_ = current_xml_lines;
        size_t code_width = 0;
        for (const string& line : code_lines)
            code_width = std::max(code_width, line.size());
        code_width = std::max<size_t>(code_width, 40);
        const size_t line_number_width = std::to_string(code_lines.size()).size();
        size_t scope_width = 0;
        for (const string& scope_line : scope_lines)
            scope_width = std::max(scope_width, scope_line.size());
        scope_width = std::max<size_t>(scope_width, 40);

        const size_t content_lines = std::max({scope_lines.size(), code_lines.size(), xml_lines.size()});
        const size_t rendered_lines = content_lines + 1;
        if (not first_run_)
            std::cout << "\033[" << (previous_rendered_lines_ + 1) << "F";
        first_run_ = false;

        std::cout << "\033[2K"
                  << "Scope" << string(scope_width - 5, ' ') << " | "
                  << string(line_number_width + 3, ' ') << "Code" << string(code_width - 4, ' ')
                  << " |   Output\n";

        const size_t line = stmt ? stmt->token().line() : -1;
        for (size_t i = 0; i < content_lines; ++i)
        {
            const bool has_scope = i < scope_lines.size();
            const bool has_code = i < code_lines.size();
            const bool is_current_line = has_code and i + 1 == line;
            const string line_number = has_code ? std::to_string(i + 1) : string{};
            std::cout << "\033[2K";
            std::cout << (has_scope ? scope_lines[i] : string{})
                      << string(scope_width - (has_scope ? scope_lines[i].size() : 0), ' ')
                      << " | "
                      << string(line_number_width - line_number.size(), ' ')
                      << (is_current_line ? string{red} + line_number + string{reset} : line_number) << " "
                      << (is_current_line ? string{red} + "●" + string{reset} : string{" "}) << " "
                      << (has_code ? highlighted_code_lines[i] : string{})
                      << string(code_width - (has_code ? code_lines[i].size() : 0), ' ')
                      << " | "
                      << (i < xml_lines.size() and xml_changed[i] ? string{green} + "+" + string{reset} : " ")
                      << " "
                      << (i < xml_lines.size() ? xml_lines[i] : string{})
                      << '\n';
        }

        // Clear rows left over when a later frame is shorter than the previous one.
        for (size_t i = rendered_lines; i < previous_rendered_lines_; ++i)
            std::cout << "\033[2K\n";

        previous_rendered_lines_ = std::max(rendered_lines, previous_rendered_lines_);

        if (not stmt)
            return;

        // 4. Wait for the user to press Enter
        string input;
        std::getline(std::cin, input);
    }
}

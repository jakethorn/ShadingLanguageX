//
// Created by jaket on 03/01/2026.
//

#ifndef FENNEC_DATA_UTILS_H
#define FENNEC_DATA_UTILS_H

#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>

using std::string;
using std::vector;
namespace fs = std::filesystem;

using namespace std::string_literals;

inline bool overwrite_data_files()
{
    return false;
}

inline fs::path get_test_data_dir()
{
    return fs::current_path() / "tests" / "data";
}

inline fs::path get_test_data(const string& relative_filepath)
{
    return get_test_data_dir() / relative_filepath;
}

inline string read_file(const fs::path& filepath)
{
    const std::ifstream file{filepath};
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

inline void write_file(const fs::path& filepath, const string& text)
{
    std::ofstream file{filepath};
    file << text;
}

inline string trim(const string& s)
{
    const std::string WHITESPACE = " \n\r\t";

    const size_t start = s.find_first_not_of(WHITESPACE);
    if (start == std::string::npos)
        return ""s;

    const size_t end = s.find_last_not_of(WHITESPACE);

    return s.substr(start, end - start + 1);
}

inline vector<string> split_lines(const string& str)
{
    vector<string> lines;
    std::stringstream ss(str);
    string line;
    while (std::getline(ss, line)) {
        lines.push_back(trim(line));
    }
    return lines;
}

inline string column_compare(const string& left_header, const string& left_text, const string& right_header, const string& right_text, const bool highlight_lines)
{
    vector<string> left_lines = split_lines(left_text);
    left_lines.insert(left_lines.cbegin(), string(left_header.size(), '-'));
    left_lines.insert(left_lines.cbegin(), left_header);

    vector<string> right_lines = split_lines(right_text);
    right_lines.insert(right_lines.cbegin(), string(right_header.size(), '-'));
    right_lines.insert(right_lines.cbegin(), right_header);

    size_t column_width = 0;
    for (const string& s : left_lines)
        column_width = std::max(column_width, s.length());
    column_width += 2;

    const size_t n_rows = std::max(left_lines.size(), right_lines.size());
    std::ostringstream output;

    const string empty = ""s;
    const string green = "\033[32m";
    const string red = "\033[31m";
    const string white = "\033[0m";
    for (size_t i = 0; i < n_rows; ++i)
    {
        const string& left_line = i < left_lines.size() ? left_lines[i] : empty;
        const string& right_right = i < right_lines.size() ? right_lines[i] : empty;

        const string& color = left_line == right_right ? green : red;

        if (i >= 2 and highlight_lines)
            output << color;

        output << left_line;

        if (left_line.length() < column_width)
        {
            output << string(column_width - left_line.length(), ' ');
        }

        output << white << "|  ";

        if (i >= 2 and highlight_lines)
            output << color;

        output << right_right << "\n";
    }

    return output.str();
}

#endif //FENNEC_DATA_UTILS_H

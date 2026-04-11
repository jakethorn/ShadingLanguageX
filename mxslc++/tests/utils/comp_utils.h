//
// Created by jaket on 30/03/2026.
//

#ifndef MXSLC_COMP_UTILS_H
#define MXSLC_COMP_UTILS_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "data_utils.h"

using std::string;
using std::vector;
namespace fs = std::filesystem;

using namespace std::string_literals;

inline string trim(const string& s)
{
    const std::string WHITESPACE = "\n\r\t";

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
    const string green  = "\033[32m";
    const string red    = "\033[31m";
    const string orange = "\033[33m"; // Terminal "Orange"
    const string white  = "\033[0m";

    auto left_lines = split_lines(left_text);
    left_lines.insert(left_lines.begin(), string(left_header.size(), '-'));
    left_lines.insert(left_lines.begin(), left_header);

    auto right_lines = split_lines(right_text);
    right_lines.insert(right_lines.begin(), string(right_header.size(), '-'));
    right_lines.insert(right_lines.begin(), right_header);

    size_t column_width = 0;
    for (const string& s : left_lines)
        column_width = std::max(column_width, s.length());
    column_width += 4; // Extra padding for safety with ANSI codes

    const size_t n_rows = std::max(left_lines.size(), right_lines.size());
    std::ostringstream output;

    for (size_t i = 0; i < n_rows; ++i)
    {
        const string& L = i < left_lines.size() ? left_lines[i] : "";
        const string& R = i < right_lines.size() ? right_lines[i] : "";

        // Row formatting lambda to handle the orange-red-orange logic
        auto append_diff = [&](const string& current, const string& other) {
            if (!highlight_lines || i < 2) {
                output << current;
                return current.length();
            }
            if (current == other) {
                output << green << current << white;
                return current.length();
            }

            // Find mismatch boundaries
            size_t p = 0;
            size_t min_v = std::min(current.length(), other.length());
            while (p < min_v && current[p] == other[p]) p++;

            size_t s = 0;
            while (s < (min_v - p) && current[current.length() - 1 - s] == other[other.length() - 1 - s]) s++;

            // Print parts: Orange Prefix | Red Middle | Orange Suffix
            output << orange << current.substr(0, p)
                   << red    << current.substr(p, current.length() - p - s)
                   << orange << current.substr(current.length() - s)
                   << white;

            return current.length();
        };

        // Left Column
        size_t len = append_diff(L, R);
        if (len < column_width)
            output << string(column_width - len, ' ');

        output << "|  ";

        // Right Column
        append_diff(R, L);
        output << "\n";
    }

    return output.str();
}

inline void print_debug_info(const fs::path& input_path, const string& actual_output, const string& expected_output)
{
    const string border = string(input_path.filename().string().size() + 4, '-');
    std::cout
    << "\n\n\n"
    << border
    << "\n"
    << "| " << input_path.filename().string() << " |"
    << "\n"
    << border
    << "\n\n"
    << column_compare("Actual Output"s, actual_output, "Expected Output"s, expected_output, true)
    << "\n\n"
    << column_compare("Actual Output"s, actual_output, "Input"s, read_file(input_path), false)
    << "\n\n\n";
}

#endif //MXSLC_COMP_UTILS_H

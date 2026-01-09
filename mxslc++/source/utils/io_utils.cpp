//
// Created by jaket on 02/11/2025.
//

#include "io_utils.h"

#include <fstream>
#include "CompileError.h"

using std::ifstream;
using std::ofstream;
using std::ostringstream;

string read_file(const fs::path& filepath)
{
    ifstream file{filepath};

    if (not file.is_open())
    {
        throw CompileError{"Cannot read file: " + filepath.string()};
    }

    ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void save_file(const fs::path& filepath, const string& text)
{
    ofstream file{filepath};

    if (not file.is_open())
    {
        throw CompileError{"Cannot save file: " + filepath.string()};
    }

    file << text;
}

fs::path get_libraries_dir()
{
    return fs::current_path() / "libraries";
}

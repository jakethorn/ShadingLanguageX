//
// Created by jaket on 26/07/2026.
//

#include "CompileOptions.h"

#include "runtime/interface.h"
#include "runtime/variables/Variable.h"
#include "utils/container_utils.h"
#include "utils/io_utils.h"
#include "utils/Logger.h"

namespace mxslc
{
    void CompileOptions::add_search_directory(fs::path dir)
    {
        if (not fs::is_directory(dir))
            Logger::warning("Search directory does not exist: " + dir.string());

        search_dirs_.push_back(std::move(dir));
    }

    void CompileOptions::add_default_search_directories(const optional<fs::path>& src_path)
    {
        container_utils::extend(search_dirs_, io_utils::get_default_search_directories(src_path));
    }

    void CompileOptions::set_current_working_directory(fs::path dir)
    {
        cwd_ = std::move(dir);
    }

    void CompileOptions::clear_search_directories()
    {
        search_dirs_.clear();
    }

    vector<fs::path> CompileOptions::search_directories() const
    {
        vector<fs::path> dirs;
        if (cwd_)
            dirs.push_back(*cwd_);
        container_utils::extend(dirs, search_dirs_);
        return dirs;
    }

    void CompileOptions::add_macro(Macro macro)
    {
        macros_.insert_or_assign(macro.name(), std::move(macro));
    }

    void CompileOptions::add_macro(string macro)
    {
        add_macro(Macro{std::move(macro)});
    }

    void CompileOptions::add_macro(string name, const string& body)
    {
        add_macro(Macro{std::move(name), body});
    }

    void CompileOptions::remove_macro(const string& name)
    {
        macros_.erase(name);
    }

    void CompileOptions::clear_macros()
    {
        macros_.clear();
    }

    const Macro& CompileOptions::get_macro(const string& name) const
    {
        if (has_macro(name))
            return macros_.at(name);
        throw CompileError{"Macro does not exist: " + name};
    }

    bool CompileOptions::has_macro(const string& name) const
    {
        return container_utils::contains(macros_, name);
    }

    void CompileOptions::add_global(string name, VarPtr value)
    {
        value->set_is_external();
        globals_.insert_or_assign(std::move(name), std::move(value));
    }

    void CompileOptions::add_global(string name, Primitive value)
    {
        add_global(std::move(name), create_variable(std::move(value)));
    }

    void CompileOptions::set_globals(unordered_map<string, VarPtr> globals)
    {
        globals_ = std::move(globals);
        for (const auto& [name, value] : globals_)
            value->set_is_external();
    }

    void CompileOptions::set_globals(const unordered_map<string, Primitive>& globals)
    {
        unordered_map<string, VarPtr> new_globals;
        new_globals.reserve(globals.size());
        for (const auto& [name, value] : globals)
            new_globals.insert_or_assign(std::move(name), create_variable(value));
        set_globals(std::move(new_globals));
    }

    void CompileOptions::clear_globals()
    {
        globals_.clear();
    }

    const VarPtr &CompileOptions::get_global(const string& name) const
    {
        if (has_global(name))
            return globals_.at(name);
        throw CompileError{"Global does not exist: " + name};
    }

    bool CompileOptions::has_global(const string& name) const
    {
        return container_utils::contains(globals_, name);
    }

    void CompileOptions::add_entry_function_argument(VarPtr value)
    {
        value->set_is_external();
        func_args_.push_back(std::move(value));
    }

    void CompileOptions::add_entry_function_argument(Primitive value)
    {
        add_entry_function_argument(create_variable(std::move(value)));
    }

    void CompileOptions::set_entry_function_arguments(vector<VarPtr> args)
    {
        func_args_ = std::move(args);
        for (const VarPtr& arg : func_args_)
            arg->set_is_external();
    }

    void CompileOptions::set_entry_function_arguments(const vector<Primitive>& args)
    {
        vector<VarPtr> new_args;
        new_args.reserve(args.size());
        for (const Primitive& arg : args)
            new_args.push_back(create_variable(arg));
        set_entry_function_arguments(std::move(new_args));
    }

    void CompileOptions::clear_entry_function_arguments()
    {
        func_args_.clear();
    }
}

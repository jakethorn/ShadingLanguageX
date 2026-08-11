//
// Created by jaket on 27/07/2026.
//

#include "utils/Logger.h"

#include <iostream>

#include "errors/CompileError.h"
#include "utils/container_utils.h"

namespace mxslc
{
    using container_utils::contains;

    void Logger::debug(const string& message)
    {
#ifndef NDEBUG
        print(Debug, message);
#endif
    }

    void Logger::info(const string& message)
    {
        print(Info, message);
    }

    void Logger::warning(const string& message)
    {
        print(Warning, message);
    }

    void Logger::error(const string& message)
    {
        print(Error, message);
    }

    void Logger::error(const CompileError& e)
    {
        error(e.what());
    }

    void Logger::assert(const bool condition, const string& message)
    {
        if (condition)
            error(message);
    }

    void Logger::disable()
    {
        disable(Debug);
        disable(Info);
        disable(Warning);
        disable(Error);
    }

    void Logger::disable(const Level level)
    {
        disabled_levels_.insert(level);
    }

    void Logger::enable()
    {
        enable(Debug);
        enable(Info);
        enable(Warning);
        enable(Error);
    }

    void Logger::enable(const Level level)
    {
        disabled_levels_.erase(level);
    }

    void Logger::print(const Level level, const string& message)
    {
        if (contains(disabled_levels_, level))
            return;

        static const string White   = "\033[0m";
        static const string Red     = "\033[31m";
        static const string Green   = "\033[32m";
        static const string Yellow  = "\033[33m";

        switch (level)
        {
            case Debug:
                std::cout << Green << "[DEBUG] " << message << White << "\n";
                break;
            case Info:
                std::cout << White << "[INFO] " << message << White << "\n";
                break;
            case Warning:
                std::clog << Yellow << "[WARNING] " << message << White << "\n";
                break;
            case Error:
                std::cerr << Red << "[ERROR] " << message << White << "\n";
                break;
            default: ;
        }
    }
}

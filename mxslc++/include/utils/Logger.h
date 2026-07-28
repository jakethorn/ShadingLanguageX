//
// Created by jaket on 27/07/2026.
//

#ifndef MXSLC_LOGGER_H
#define MXSLC_LOGGER_H

#include "common.h"

namespace mxslc
{
    class CompileError;

    class Logger
    {
    public:
        enum Level
        {
            Debug,
            Info,
            Warning,
            Error
        };

        static void debug(const string& message);
        static void info(const string& message);
        static void warning(const string& message);
        static void error(const string& message);
        static void error(const CompileError& e);
        static void assert(bool condition, const string& message);

        static void disable();
        static void disable(Level level);

        static void enable();
        static void enable(Level level);

    private:
        static void print(Level level, const string& message);

        static unordered_set<Level> disabled_levels_;
    };
}

#endif //MXSLC_LOGGER_H

//
// Created by jaket on 10/08/2026.
//

#ifndef MXSLC_DEBUGGER_H
#define MXSLC_DEBUGGER_H

#include "common.h"

namespace mxslc::statements
{
    class Statement;
}

namespace mxslc::debug
{
    class Debugger
    {
    public:
        static Debugger& create(string code);
        static Debugger& get();
        static bool is_enabled();

        void next_statement(const Statement& stmt);

    private:
        explicit Debugger(string code);

        string code_;
        vector<string> previous_xml_lines_;
        bool first_run_{true};

        static unique_ptr<Debugger> instance_;
    };
}

#endif //MXSLC_DEBUGGER_H

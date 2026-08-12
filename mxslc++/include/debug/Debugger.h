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
        static Debugger& create(string code, optional<fs::path> src_path = std::nullopt);
        static Debugger& get();
        static bool is_enabled();
        static void close();

        void next_statement(const Statement* stmt);

    private:
        explicit Debugger(string code, optional<fs::path> src_path);

        string code_;
        optional<fs::path> src_path_;

        vector<string> previous_xml_lines_;
        bool first_run_{true};
        size_t previous_rendered_lines_{0};

        static unique_ptr<Debugger> instance_;
    };
}

#endif //MXSLC_DEBUGGER_H

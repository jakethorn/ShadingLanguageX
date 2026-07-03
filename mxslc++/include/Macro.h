//
// Created by jaket on 02/07/2026.
//

#ifndef MXSLC_MACRO_H
#define MXSLC_MACRO_H

#include <string>

class Macro
{
public:
    Macro(std::string name) : name_{std::move(name)} { }
    Macro(std::string name, std::string body) : name_{std::move(name)}, body_{std::move(body)} { }


    const std::string& name() const { return name_; }
    const std::string& body() const { return body_; }

private:
    std::string name_;
    std::string body_;
};

#endif //MXSLC_MACRO_H

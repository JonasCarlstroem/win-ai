#pragma once

#include <string>
#include <iostream>
#include <sstream>

struct output {
    std::string header;

    output() : header() {}
    output(const char* header) : header(header) {}

    void out(const std::string& msg) {
        std::ostringstream oss;
        p_header(oss);

        oss << msg << std::endl;
        std::cout << oss.str();
    }

    template<typename... T>
    void out(const T&... args) {
        std::ostringstream oss;
        p_header(oss);

        ((oss << args), ...) << std::endl;
        std::cout << oss.str();
    }

    void err(const std::string& msg) {
        std::ostringstream oss;
        p_header(oss);

        oss << msg << std::endl;
        std::cerr << oss.str();
    }

    template<typename... T>
    void err(const T&... args) {
        std::ostringstream oss;
        p_header(oss);

        ((oss << args), ...) << std::endl;
        std::cerr << oss.str();
    }

    output& operator()(const std::string& msg) {
        out(msg);
        return *this;
    }

    template<typename... T>
    output& operator()(const T&... args) {
        out(args...);
        return *this;
    }

private:
    void p_header(std::ostream& os) {
        if (!header.empty()) {
            os << "[" << header << "] ";
        }
    }
};
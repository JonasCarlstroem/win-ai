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

inline std::string trim(const std::string& s) {
    auto start = s.find_first_not_of(" \t\n\r");
    auto end = s.find_last_not_of(" \t\n\r");

    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

inline std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        result.push_back(item);
    }

    return result;
}

inline std::string join(const std::vector<std::string>& parts, const std::string& sep) {
    std::ostringstream os;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i > 0) os << sep;
        os << parts[i];
    }

    return os.str();
}
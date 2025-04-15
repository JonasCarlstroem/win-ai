#pragma once

#include <string>
#include <iostream>
#include <chrono>
#include <thread>
#include "util.h"

//using c_clock = std::chrono::steady_clock;
//using c_time_point = std::chrono::steady_clock::time_point;
using namespace std;

class timer : public output {
public:
    timer() : is_running(false), output("Timer") {}
    timer(const std::string& header) : is_running(false), output(header.c_str()) {}

    void display_elapsed_seconds(const std::string& caption) {
        out(caption, " ", elapsed_seconds(), "s");
    }

    template<typename... T>
    void display_elapsed_seconds(T&&... args) {
        out((args, ...), " ", elapsed_seconds(), "s");
    }

    void start(const std::string& caption = "") {
        start_time = chrono::steady_clock::now();
        is_running = true;

        //if (!caption.empty()) {
        //    out(caption);
        //}
        //else {
        //    out("Timer started");
        //}
    }

    void stop(const std::string& caption = "") {
        if (is_running) {
            end_time = chrono::steady_clock::now();
            is_running = false;

            //if (!caption.empty()) {
            //    out(caption);
            //}
            //else {
            //    out("Timer stopped");
            //}
        }
    }

    void reset() {
        is_running = false;
        start_time = {};
        end_time = {};
    }

    void restart() {
        reset();
        start();
    }

    double elapsed_seconds() const {
        if (is_running) {
            return chrono::duration<double>(
                chrono::steady_clock::now() - start_time
            ).count();
        } 
        else {
            return chrono::duration<double>(end_time - start_time).count();
        }
    }

private:
    chrono::steady_clock::time_point start_time;
    chrono::steady_clock::time_point end_time;
    
    bool is_running;
};
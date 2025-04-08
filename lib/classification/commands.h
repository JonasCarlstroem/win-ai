#pragma once
#include <string>
#include <unordered_map>
#include <functional>
#include <iostream>
#include <cstdlib>
#include <fstream>

using namespace std;

enum class command_type {
    launch_process,
    read_file,
    chat,
    unknown
};

struct intent {
    string name;
    command_type type;
    function<void()> action;
};

inline const unordered_map<string, intent> intent_map = {
    {
        "open_browser", {
            "open_browser",
            command_type::launch_process,
            []() {}
        }
    },
    {
        "open_notepad", {
            "open_notepad",
            command_type::launch_process,
            []() {}
        }
    },
    {
        "read_file", {
            "read_file",
            command_type::read_file,
            []() {
                ifstream file("example.txt");
                if (!file) {
                    cout << "Failed to open file.\n";
                    return;
                }

                cout << "--- File contents ---\n";
                string line;
                while (getline(file, line)) {
                    cout << line << '\n';
                }
            }
        }
    },
    {
        "chat", {
            "chat",
            command_type::chat,
            []() {
                cout << "[AI assistant] Chat is currently not available.";
            }
        }
    }
};

inline void execute_intent(const string& intent_name) {
    auto it = intent_map.find(intent_name);
    if (it != intent_map.end()) {
        const intent& intent_ = it->second;
        cout << "[Intent] " << intent_.name << " (" << static_cast<int>(intent_.type) << ")\n";
        intent_.action();
    }
    else {
        cout << "Unknown intent: " << intent_name << "\n";
    }
}
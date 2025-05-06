#include <iostream>
#include <fstream>
#include <filesystem>

using namespace std;
namespace fs = filesystem;

template<typename... T>
void out(T... args) {
    ((cout << args), ...) << endl;
}

template<typename... Ts, size_t... N>
void str(Ts (&... args)[N]) {
    size_t total_length = (std::strlen(args) + ...);

    char* result = static_cast<char*>(std::malloc(total_length + 1));
    if (!result) return nullptr;
    
    char* current = result;
}

class parser {
private:
    const char* _fileName;
    const char* _content;

public:
    parser(const char* fileName) : _fileName(fileName) {
        if (!fs::exists(fileName)) throw exception("File not found");
        str("Hello", "Why");
    }
};

int main() {
    parser p("thesaurus.txt");
    return 0;
}
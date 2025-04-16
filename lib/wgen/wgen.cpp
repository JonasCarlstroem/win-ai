#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <regex>
#include <set>
#include <filesystem>
#include "../util.h"
#include "lex_parser.h"

namespace fs = std::filesystem;

int main() {
    output out("wgen");

    bool index = true;
    bool parse = false;

    if (!fs::exists("thesaurus.txt")) {
        out.err("File not found...");
        return 1;
    }

    lex_parser parser("thesaurus.txt", out);
    parser.init();

    //parser.extract_indices();
    parser.extract_indices2();
    parser.display_toc();

    parser.display_section(1);

    return 0;
}
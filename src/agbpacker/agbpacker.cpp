// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <argp.h>
#include <iostream>
#include <filesystem>
#include <stdexcept>
#include <string>

import argpppp;

namespace
{

// TODO: put this into some sort of tools library/module => argpppp
auto program_name(char* argv0)
{
    namespace fs = std::filesystem;
    return fs::path(argv0).stem().string();
}

}

// TODO: put this into some sort of argpppp library/module
// TODO: this could use the program_name thing above too, no?
// TODO: features
//       * Does not terminate your application, unless you want it to
//       * Works with glibc argp or argp_standalone
//       * Properly prints your program name, in any damn case
/*namespace argpppp
{

class parser final
{
public:
private:
};

}*/

int main(int argc, char** argv)
{
    try
    {
        argpppp::parser parser;
        parser.parse(argc, argv);
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        std::cerr << program_name(argv[0]) << ": " << e.what() << "\n";
        return EXIT_FAILURE;
    }
}

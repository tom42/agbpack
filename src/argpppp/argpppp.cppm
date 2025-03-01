// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <argp.h>
#include <string>

// TODO: API documentation
//       * program_name (once it's settled down)
// TODO: features to make available:
//       * argp_program_version (global variable)
//       * argp_program_bug_address (global variable)
//       * doc string (goes into parser definition => want this to be dynamic!)

export module argpppp;

namespace argpppp
{

export std::string program_name(const char* argv0);

// TODO: this could use the program_name thing above too, no?
// TODO: features
//       * Does not terminate your application, unless you want it to
//       * Properly prints your program name, in any damn case
export class parser final
{
public:
    void parse(int argc, char** argv)
    {
        argp_parse(nullptr, argc, argv, 0, nullptr, this);
    }
private:
};

}

// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <argp.h>
#include <string>

export module argpppp;

namespace argpppp
{

export std::string program_name(const char* argv0);

// TODO: this could use the program_name thing above too, no?
// TODO: features
//       * Does not terminate your application, unless you want it to
//       * Works with glibc argp or argp-standalone
//       * Properly prints your program name, in any damn case
//       * Start writing README.md, so we don't forget about it.
export class parser final
{
public:
    void parse(int argc, char** argv)
    {
        argp_parse(nullptr, argc, argv, 0, nullptr, nullptr);
    }
private:
};

}

// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <argp.h>
#include <string>

export module argpppp;

namespace argpppp
{

export std::string program_name(const char* argv0);

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

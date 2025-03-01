// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <argp.h>

export module argpppp;

namespace argpppp
{

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

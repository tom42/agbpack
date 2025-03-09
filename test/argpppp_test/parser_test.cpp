// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <cstring>
#include <string>
#include <vector>

import argpppp;

namespace argpppp_test
{

namespace
{

std::vector<char> make_arg(const char* s)
{
    // Copy including terminating zero
    return std::vector<char>(s, s + strlen(s) + 1);
}

}

TEST_CASE("parser_test")
{
    argpppp::parser parser;

    SECTION("Exceptions are propagated to caller")
    {
        auto arg0 = make_arg("program_name");
        auto arg1 = make_arg("-s");
        std::vector<char*> argv;
        argv.push_back(arg0.data());
        argv.push_back(arg1.data());

        // TODO: allow supplying a callback. The callback should throw an exception, which we expect to catch
        parser.add_option({ {}, 's' });

        // TODO: turn this into some sort of run_parse(parser, "command line without program name") function
        //parser.parse(static_cast<int>(argv.size()), argv.data());
    }
}

}

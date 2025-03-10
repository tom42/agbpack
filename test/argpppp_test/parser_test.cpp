// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <cstring>
#include <stdexcept>
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

    SECTION("Exceptions abort parsing and are propagated to caller")
    {
        auto arg0 = make_arg("program_name");
        auto arg1 = make_arg("-a");
        auto arg2 = make_arg("-b");
        std::vector<char*> argv;
        argv.push_back(arg0.data());
        argv.push_back(arg1.data());
        argv.push_back(arg2.data());

        parser.add_option({ {}, 'a' }, []{ throw std::runtime_error("This exception should occur."); });
        parser.add_option({ {}, 'b' }, []{ throw std::runtime_error("This exception should not occur."); });

        CHECK_THROWS_MATCHES(
            parser.parse(static_cast<int>(argv.size()), argv.data()),
            std::runtime_error,
            Catch::Matchers::Message("This exception should occur."));
    }
}

}

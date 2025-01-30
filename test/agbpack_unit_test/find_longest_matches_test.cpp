// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>

import agbpack;
import agbpack_unit_testkit;

namespace agbpack_unit_test
{

using agbpack::match;
using byte_vector = std::vector<unsigned char>;
using match_vector = std::vector<match>;

namespace
{

// TODO: do we really want this to have the same name as the production function?
auto find_longest_matches(const std::string& s)
{
    return agbpack::find_longest_matches(byte_vector(begin(s), end(s)));
}

}

TEST_CASE("find_longest_matches_test")
{
    // TODO: test something here
    // TODO: screw that: we're not going to have a class for this: it feels silly, somehow
    //       * For starters and to keep things simple, then, organize the encoder as such:
    //         0) Read in the entire input (do we have a utility function for this?)
    //         1) Call create_maximum_match_lengths()  <=  This is what we be testing here
    //         2) Call create_chosen_match_lengths()
    //         3) Call encode()
    SECTION("Empty input")
    {
        CHECK(find_longest_matches("") == match_vector());
    }

    SECTION("One byte")
    {
        // TODO: create byte_vector from const char*
        // TODO: check it returns the right match
        CHECK(find_longest_matches("a") == match_vector{match(42, 43)});
    }
}

}

// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <vector>

import agbpack;
import agbpack_unit_testkit;

namespace agbpack_unit_test
{

using agbpack::find_longest_matches;
using agbpack::match;
using byte_vector = std::vector<unsigned char>;
using match_vector = std::vector<match>;

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
        CHECK(find_longest_matches(byte_vector()) == match_vector());
    }

    SECTION("One byte")
    {
        // TODO: create byte_vector from const char*
        // TODO: check it returns the right match
    }
}

}

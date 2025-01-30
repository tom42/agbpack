// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

import agbpack;
import agbpack_unit_testkit;

namespace agbpack_unit_test
{

using agbpack::maximum_match_length_table;

TEST_CASE("maximum_match_length_table_test")
{
    // TODO: test something here
    // TODO: screw that: we're not going to have a class for this: it feels silly, somehow
    //       * For starters and to keep things simple, then, organize the encoder as such:
    //         0) Read in the entire input (do we have a utility function for this?)
    //         1) Call create_maximum_match_lengths()  <=  This is what we be testing here
    //         2) Call create_chosen_match_lengths()
    //         3) Call encode()
}

}

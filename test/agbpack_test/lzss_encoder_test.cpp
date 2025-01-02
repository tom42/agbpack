// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

import agbpack;

namespace agbpack_test
{

TEST_CASE("lzss_encoder_test")
{
    agbpack::lzss_encoder encoder;
    // TODO: use standard fixture and set test input directory, I guess

    SECTION("Invalid options")
    {
        // TODO: set invalid options on encoder and catch exception. Problem: encoder does not even exist yet
    }
}

}

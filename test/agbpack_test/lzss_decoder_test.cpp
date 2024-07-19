// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include "testdata.hpp"

import agbpack;

TEST_CASE("lzss_decoder_test")
{
    agbpack::lzss_decoder decoder;
    (void)decoder; // TODO: remove

    SECTION("Invalid input")
    {
        auto encoded_file = GENERATE("lzss.bad.eof-inside-header.txt.encoded");

        CHECK_THROWS_AS(agbpack_test::decode_file(decoder, encoded_file), agbpack::bad_encoded_data);
    }
}

// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

import agbpack;
#include <catch2/catch_test_macros.hpp>

TEST_CASE("rle_decoder")
{
    // TODO: decode a first simple file: maybe one without any compression at all
    // TODO: decode a file with compression
    // TODO: decode a zero length file
    // TODO: pathological stuff: malformed streams, too long, too short, malformed header
    // TODO: need to figure out a way how to make data files available to tests:
    //       * Bake path into test suite
    //       * Copy data files to output directory
    agbpack::rle_decoder decoder;

    // TODO: remove
    (void)decoder;
}
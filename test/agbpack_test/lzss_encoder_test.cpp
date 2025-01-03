// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include "testdata.hpp"

import agbpack;

namespace agbpack_test
{

TEST_CASE_METHOD(test_data_fixture, "lzss_encoder_test")
{
    agbpack::lzss_encoder encoder;
    agbpack::lzss_decoder decoder;
    set_test_data_directory("lzss_encoder");

    // TODO: remove
    (void)encoder;(void)decoder;

    // TODO: add tests: successful encoding: encode stuff and check against decoder
    //       * First test: zero bytes of input, no? Where to get reference data from? Do we need reference data?
    //         * No we don't, but maybe check whether e.g. CUE can deal with zero bytes of input
}

}

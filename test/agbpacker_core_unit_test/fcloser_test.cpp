// SPDX-FileCopyrightText: 2026 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <string>
#include "agbpack_test_config.hpp"

import agbpacker_core;

namespace agbpacker_core_unit_test
{

using agbpacker_core::fcloser;

namespace
{

std::string full_path(std::string_view basename)
{
    namespace fs = std::filesystem;
    return (fs::path(agbpack_test::testdata_directory) / fs::path(basename)).string();
}

}

TEST_CASE("fcloser_test")
{
    SECTION("fcloser, open nonexistent file")
    {
        CHECK_THROWS_AS(
            fcloser::open(full_path("file/nonexistent.txt"), "r"),
            std::system_error);
    }

    SECTION("fcloser, open existing file")
    {
        auto fp = fcloser::open(full_path("file/file.txt"), "r");
        CHECK(std::ftell(fp.get()) == 0);
    }
}

}

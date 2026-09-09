// SPDX-FileCopyrightText: 2026 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <cstdio>
#include <filesystem>
#include <string_view>
#include <system_error>
#include "../agbpack_test_config.hpp" // TODO: this is not exactly pretty => should set proper path in CMakeLists.txt to begin with

import agbpacker_core;

namespace agbpacker_core_unit_test
{

using agbpacker_core::fcloser;

namespace
{

std::string full_path(std::string_view basename)
{
    // TODO: agbpack_test_testdata_directory is in agbpack_test specific namespace
    //       * do we care?
    //       * do we even need it to be in a namespace
    namespace fs = std::filesystem;
    return (fs::path(agbpack_test::agbpack_test_testdata_directory) / fs::path(basename)).string();
}

}

TEST_CASE("file_test")
{
    SECTION("fcloser, open nonexistent file")
    {
        CHECK_THROWS_AS(
            fcloser::open(full_path("file/nonexistent.txt"), "rb"),
            std::system_error);
    }

    SECTION("fcloser, open existing file")
    {
        auto fp = fcloser::open(full_path("file/file.txt"), "rb");
        CHECK(std::ftell(fp.get()) == 0);
    }
}

}

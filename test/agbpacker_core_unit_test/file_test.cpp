// SPDX-FileCopyrightText: 2026 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <cstddef>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>
#include "agbpack_test_config.hpp"

import agbpacker_core;

namespace agbpacker_core_unit_test
{

using agbpacker_core::file;
using agbpacker_core::seek_origin;

namespace
{

std::string full_path(std::string_view basename)
{
    namespace fs = std::filesystem;
    return (fs::path(agbpack_test::testdata_directory) / fs::path(basename)).string();
}

}

TEST_CASE("file_test")
{
    SECTION("file, open nonexistent file")
    {
        CHECK_THROWS_AS(
            file::open(full_path("file/nonexistent.txt"), "r"),
            std::system_error);
    }

    SECTION("file, open existing file")
    {
        auto file = file::open(full_path("file/file.txt"), "r");
        CHECK(file.tell() == 0);
    }

    SECTION("seek and tell")
    {
        auto file = file::open(full_path("file/file.txt"), "r");
        file.seek(0, seek_origin::end);
        CHECK(file.tell() == 12);

        file.seek(-1, seek_origin::cur);
        CHECK(file.tell() == 11);

        file.seek(-2, seek_origin::cur);
        CHECK(file.tell() == 9);

        file.seek(1, seek_origin::set);
        CHECK(file.tell() == 1);
    }

    SECTION("size")
    {
        auto file = file::open(full_path("file/file.txt"), "r");
        file.seek(3, seek_origin::set);

        CHECK(file.size() == 12);
        CHECK(file.tell() == 3);
    }

    SECTION("read")
    {
        auto file = file::open(full_path("file/file.txt"), "r");
        const std::size_t nbytes = 3;
        std::vector<char> buffer(nbytes);

        file.read(buffer.data(), nbytes);

        CHECK(buffer == std::vector<char>{ 'f', 'i', 'l' });
    }

    SECTION("read past end of file")
    {
        auto file = file::open(full_path("file/file.txt"), "r");
        const std::size_t nbytes = 13;
        std::vector<char> buffer(nbytes);

        CHECK_THROWS_MATCHES(
            file.read(buffer.data(), nbytes),
            std::logic_error,
            Catch::Matchers::Message("read past end of file"));
    }

    SECTION("read_all_bytes")
    {
        auto all_bytes = file::read_all_bytes(full_path("file/file.txt"));
        CHECK(all_bytes == std::vector<unsigned char>{ 'f', 'i', 'l', 'e', ' ', 'c', 'o', 'n', 't', 'e', 'n', 't'});
    }

    SECTION("write_all_bytes")
    {
        // TODO: write test for write_all_bytes. Problem:
        //       * we need some input data (well that's just a string/vector, no?)
        //       * we need a place to write it to (well that would be CWD for starters, or not? or maybe use some CMake binary directory?)
    }
}

}

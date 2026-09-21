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
namespace fs = std::filesystem;

namespace
{

std::string full_path(std::string_view basename)
{
    return (fs::path(agbpack_test::testdata_directory) / fs::path(basename)).string();
}

// TODO: this is going to fly apart as soon as we add another test. Should have a class test_directory which is able to generate random filenames
std::string output_filename(std::string_view basename)
{
    fs::create_directories(agbpack_test::testoutput_directory);
    return (fs::path(agbpack_test::testoutput_directory) / fs::path(basename)).string();
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

    SECTION("write")
    {
        auto file = file::open(output_filename("write.dat"), "w+b");

        file.write("d", 1);
        file.write("ata", 3);

        //file.seek(0, seek_origin::set); // TODO: uncomment this to make the test pass. Oddly enough, without this we do not read past EOF on windows, but read all 0 bytes. Why?
        std::vector<char> buffer(4);
        file.read(buffer.data(), 4);

        CHECK(buffer == std::vector<char>{ 'd', 'a', 't', 'a' });

        // TODO: write data
        // TODO: write some more data
        // TODO: read, should all be good?
    }

    SECTION("read_all_bytes")
    {
        auto all_bytes = file::read_all_bytes(full_path("file/file.txt"));
        CHECK(all_bytes == std::vector<unsigned char>{ 'f', 'i', 'l', 'e', ' ', 'c', 'o', 'n', 't', 'e', 'n', 't'});
    }

    SECTION("write_all_bytes")
    {
        const std::vector<unsigned char> data{ 'd', 'a', 't', 'a' };
        const auto filename = output_filename("write_all_bytes.dat");

        file::write_all_bytes(filename, data);

        CHECK(file::read_all_bytes(filename) == data);
    }
}

}

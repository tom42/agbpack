// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <cstddef>
#include <stdexcept>

import agbpack;
import agbpack_unit_testkit;

namespace agbpack_unit_test
{

// TODO: unify spelling: sometimes we use bytevector, sometimes byte_vector
using bytevector = std::vector<unsigned char>;

// TODO: when done, see which usings we still need
using agbpack::compression_type;
using agbpack::delta_options;
using agbpack::encode_exception;
using agbpack::header;
using agbpack::header2;
using agbpack::huffman_options;
using agbpack::maximum_uncompressed_size;
using namespace std::string_literals;

TEST_CASE("header_test_old") // TODO: remove once class is replaced
{
    SECTION("create, valid size")
    {
        auto valid_size = GENERATE(0u, maximum_uncompressed_size);

        CHECK(header::create(huffman_options::h8, valid_size).uncompressed_size() == valid_size);
    }

    SECTION("create, invalid size")
    {
        CHECK_THROWS_MATCHES(
            header::create(huffman_options::h8, maximum_uncompressed_size + 1),
            encode_exception,
            Catch::Matchers::Message("data to encode is too big"));

#if SIZE_MAX > 0xffffffff
        CHECK_THROWS_MATCHES(
            header::create(huffman_options::h8, 0x100000000),
            encode_exception,
            Catch::Matchers::Message("data to encode is too big"));
#endif
    }

    SECTION("to_uint32_t")
    {
        auto header = header::create(huffman_options::h8, 0x123456);

        CHECK(header.to_uint32_t() == 0x12345628);
    }
}

TEST_CASE("header_test")
{
    SECTION("try_parse from span, enough data")
    {
        const auto data = GENERATE(
            bytevector{ 0x28, 0x56, 0x34, 0x12 },
            bytevector{ 0x28, 0x56, 0x34, 0x12, 0x99 });

        const auto header = header2::try_parse(data).value();

        CHECK(header.type() == compression_type::huffman);
        CHECK(header.options<huffman_options>() == huffman_options::h8);
        CHECK(header.uncompressed_size() == 0x123456);
    }

    SECTION("try_parse from span, not enough data")
    {
        const bytevector data = { 0x28, 0x56, 0x34 };

        const auto header = header2::try_parse(data);

        CHECK(header.has_value() == false);
        CHECK(header.error() == "premature end of data"s);
    }

    SECTION("try_parse from uint32_t, huffman header")
    {
        const auto header = header2::try_parse(0x12345628).value();

        CHECK(header.type() == compression_type::huffman);
        CHECK(header.options<huffman_options>() == huffman_options::h8);
        CHECK(header.uncompressed_size() == 0x123456);
    }

    SECTION("try_parse from uint32_t, delta header")
    {
        const auto header = header2::try_parse(0xabcdef81).value();

        CHECK(header.type() == compression_type::delta);
        CHECK(header.options<delta_options>() == delta_options::delta8);
        // TODO: check size
    }
}

}

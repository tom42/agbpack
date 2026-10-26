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
    SECTION("create, invalid size")
    {
#if SIZE_MAX > 0xffffffff
        CHECK_THROWS_MATCHES(
            header::create(huffman_options::h8, 0x100000000),
            encode_exception,
            Catch::Matchers::Message("data to encode is too big"));
#endif
    }
}

TEST_CASE("header_test")
{
    SECTION("create_lzss")
    {
        const auto header = header2::create_lzss(0xabcdef);

        CHECK(header.type() == compression_type::lzss);
        CHECK(header.uncompressed_size() == 0xabcdef);
    }

    SECTION("create_huffman")
    {
        const auto header = header2::create_huffman(huffman_options::h4, 0x123456);

        CHECK(header.type() == compression_type::huffman);
        CHECK(header.options<huffman_options>() == huffman_options::h4);
        CHECK(header.uncompressed_size() == 0x123456);
    }

    SECTION("create_rle")
    {
        const auto header = header2::create_rle(0x234567);

        CHECK(header.type() == compression_type::rle);
        CHECK(header.uncompressed_size() == 0x234567);
    }

    SECTION("create_delta")
    {
        const auto header = header2::create_delta(delta_options::delta8, 0x345678);

        CHECK(header.type() == compression_type::delta);
        CHECK(header.options<delta_options>() == delta_options::delta8);
        CHECK(header.uncompressed_size() == 0x345678);
    }

    SECTION("create header with valid size")
    {
        auto valid_size = GENERATE(0u, maximum_uncompressed_size);

        CHECK(header2::create_lzss(valid_size).uncompressed_size() == valid_size);
    }

    SECTION("create header with invalid size")
    {
        // TODO: more generic message? No "encode" in message?
        CHECK_THROWS_MATCHES(
            header2::create_lzss(maximum_uncompressed_size + 1),
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
        CHECK(header2::create_lzss(0x456789).to_uint32_t() == 0x45678910);
        CHECK(header2::create_huffman(huffman_options::h8, 0x567890).to_uint32_t() == 0x56789028);
    }

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

        const auto parse_result = header2::try_parse(data);

        CHECK(parse_result.has_value() == false);
        CHECK(parse_result.error() == "premature end of data"s);
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
        CHECK(header.uncompressed_size() == 0xabcdef);
    }

    SECTION("try_parse from uint32_t, invalid compression type")
    {
        const auto parse_result = header2::try_parse(0);

        CHECK(parse_result.has_value() == false);
        CHECK(parse_result.error() == "invalid compression type"s);
    }

    SECTION("try_parse from uint32_t, invalid huffman compression options")
    {
        const auto parse_result = header2::try_parse(0x2f);

        CHECK(parse_result.has_value() == false);
        CHECK(parse_result.error() == "invalid compression options"s);
    }

    SECTION("try_parse from uint32_t, invalid delta compression options")
    {
        const auto parse_result = header2::try_parse(0x80);

        CHECK(parse_result.has_value() == false);
        CHECK(parse_result.error() == "invalid compression options"s);
    }
}

}

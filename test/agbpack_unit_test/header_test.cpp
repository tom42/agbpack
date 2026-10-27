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

using agbpack::compression_type;
using agbpack::delta_options;
using agbpack::encode_exception;
using agbpack::header;
using agbpack::huffman_options;
using agbpack::maximum_uncompressed_size;
using namespace std::string_literals;

using byte_vector = std::vector<unsigned char>;

TEST_CASE("header_test")
{
    SECTION("create_lzss")
    {
        const auto header = header::create_lzss(0xabcdef);

        CHECK(header.type() == compression_type::lzss);
        CHECK(header.uncompressed_size() == 0xabcdef);
    }

    SECTION("create_huffman")
    {
        const auto header = header::create_huffman(huffman_options::h4, 0x123456);

        CHECK(header.type() == compression_type::huffman);
        CHECK(header.options<huffman_options>() == huffman_options::h4);
        CHECK(header.uncompressed_size() == 0x123456);
    }

    SECTION("create_rle")
    {
        const auto header = header::create_rle(0x234567);

        CHECK(header.type() == compression_type::rle);
        CHECK(header.uncompressed_size() == 0x234567);
    }

    SECTION("create_delta")
    {
        const auto header = header::create_delta(delta_options::delta8, 0x345678);

        CHECK(header.type() == compression_type::delta);
        CHECK(header.options<delta_options>() == delta_options::delta8);
        CHECK(header.uncompressed_size() == 0x345678);
    }

    SECTION("create header with valid size")
    {
        auto valid_size = GENERATE(0u, maximum_uncompressed_size);

        CHECK(header::create_lzss(valid_size).uncompressed_size() == valid_size);
    }

    SECTION("create header with invalid size")
    {
        CHECK_THROWS_MATCHES(
            header::create_lzss(maximum_uncompressed_size + 1),
            encode_exception,
            Catch::Matchers::Message("uncompressed size is too big"));

#if SIZE_MAX > 0xffffffff
        CHECK_THROWS_MATCHES(
            header::create_lzss(0x100000000),
            encode_exception,
            Catch::Matchers::Message("uncompressed size is too big"));
#endif
    }

    SECTION("create header with invalid options")
    {
        CHECK_THROWS_MATCHES(
            header::create_huffman(static_cast<huffman_options>(15), 0),
            std::invalid_argument,
            Catch::Matchers::Message("invalid compression options"));
    }

    SECTION("to_uint32_t")
    {
        CHECK(header::create_lzss(0x456789).to_uint32_t() == 0x45678910);
        CHECK(header::create_huffman(huffman_options::h8, 0x567890).to_uint32_t() == 0x56789028);
    }

    SECTION("try_parse from span, enough data")
    {
        const auto data = GENERATE(
            byte_vector{ 0x28, 0x56, 0x34, 0x12 },
            byte_vector{ 0x28, 0x56, 0x34, 0x12, 0x99 });

        const auto header = header::try_parse(data).value();

        CHECK(header.type() == compression_type::huffman);
        CHECK(header.options<huffman_options>() == huffman_options::h8);
        CHECK(header.uncompressed_size() == 0x123456);
    }

    SECTION("try_parse from span, not enough data")
    {
        const byte_vector data = { 0x28, 0x56, 0x34 };

        const auto parse_result = header::try_parse(data);

        CHECK(parse_result.has_value() == false);
        CHECK(parse_result.error() == "premature end of data"s);
    }

    SECTION("try_parse from uint32_t, huffman header")
    {
        const auto header = header::try_parse(0x12345628).value();

        CHECK(header.type() == compression_type::huffman);
        CHECK(header.options<huffman_options>() == huffman_options::h8);
        CHECK(header.uncompressed_size() == 0x123456);
    }

    SECTION("try_parse from uint32_t, delta header")
    {
        const auto header = header::try_parse(0xabcdef81).value();

        CHECK(header.type() == compression_type::delta);
        CHECK(header.options<delta_options>() == delta_options::delta8);
        CHECK(header.uncompressed_size() == 0xabcdef);
    }

    SECTION("try_parse from uint32_t, invalid compression type")
    {
        const auto parse_result = header::try_parse(0);

        CHECK(parse_result.has_value() == false);
        CHECK(parse_result.error() == "invalid compression type"s);
    }

    SECTION("try_parse from uint32_t, invalid huffman compression options")
    {
        const auto parse_result = header::try_parse(0x2f);

        CHECK(parse_result.has_value() == false);
        CHECK(parse_result.error() == "invalid compression options"s);
    }

    SECTION("try_parse from uint32_t, invalid delta compression options")
    {
        const auto parse_result = header::try_parse(0x80);

        CHECK(parse_result.has_value() == false);
        CHECK(parse_result.error() == "invalid compression options"s);
    }
}

}

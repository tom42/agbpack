// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <cstddef>
#include <filesystem>
#include <format>
#include <string>
#include "testdata.hpp"

import agbpack;

namespace agbpack_test
{

using pair = std::pair<const char*, const char*>;
using size_t = std::size_t;
using string = std::string;

namespace
{

size_t guess_uncompressed_size(const string& basename, const test_data_fixture& fixture)
{
    // Get size of uncompressed data from .decoded file if it exists.
    auto decoded_file_path = fixture.get_decoded_file_path(basename);
    if (std::filesystem::exists(decoded_file_path))
    {
        return get_file_size(decoded_file_path);
    }

    // No .decoded file. Then try reading uncompressed size from .encoded file.
    auto encoded_file_content = fixture.read_encoded_file(basename);
    if (encoded_file_content.size() >= 4)
    {
        size_t uncompressed_size = encoded_file_content[1];
        uncompressed_size += encoded_file_content[2] * 256u;
        uncompressed_size += encoded_file_content[3] * 256u * 256u;
        return uncompressed_size;
    }

    // Cannot even guess size from header.
    // Assume encoded file is so broken that no output will be created.
    return 0;
}

template <typename TDecoder>
std::vector<unsigned char> decode_file_to_random_access_iterator(TDecoder& decoder, const string& basename, const test_data_fixture& fixture)
{
    const auto encoded_data = fixture.read_encoded_file(basename);
    std::vector<unsigned char> decoded_data(guess_uncompressed_size(basename, fixture));
    decoder.decode(encoded_data.begin(), encoded_data.end(), decoded_data.begin());
    return decoded_data;
}

}

TEST_CASE_METHOD(test_data_fixture, "lzss_decoder_test")
{
    agbpack::lzss_decoder decoder;
    set_test_data_directory("lzss_decoder");

    SECTION("Valid input")
    {
        const auto filename = GENERATE(
            "lzss.good.1-literal.txt",
            "lzss.good.8-literals.txt",
            "lzss.good.17-literals.txt",
            "lzss.good.reference-1.txt",
            "lzss.good.reference-2.txt",
            "lzss.good.zero-length-file.txt",
            "lzss.good.reference-with-minimum-offset.txt",
            "lzss.good.reference-with-maximum-offset.txt",
            "lzss.good.reference-with-minimum-match-length.txt",
            "lzss.good.reference-with-maximum-match-length.txt",
            "lzss.good.literals-and-references.txt");
        const auto expected_decoded_data = read_decoded_file(filename);

        CHECK(decode_file(decoder, filename) == expected_decoded_data);
        CHECK(decode_file_to_random_access_iterator(decoder, filename, *this) == expected_decoded_data);
    }

    SECTION("Invalid input")
    {
        const auto [filename, expected_exception_message] = GENERATE(
            pair("lzss.bad.eof-inside-header.txt", "encoded data is corrupt"),
            pair("lzss.bad.eof-at-flag-byte.txt", "encoded data is corrupt"),
            pair("lzss.bad.eof-at-reference-byte-1.txt", "encoded data is corrupt"),
            pair("lzss.bad.eof-at-reference-byte-2.txt", "encoded data is corrupt"),
            pair("lzss.bad.eof-at-literal.txt", "encoded data is corrupt"),
            pair("lzss.bad.reference-goes-past-decompressed-size.txt", "encoded data is corrupt"),
            pair("lzss.bad.invalid-compression-type-in-header.txt", "encoded data is corrupt"),
            pair("lzss.bad.valid-but-unexpected-compression-type-in-header.txt", "encoded data is corrupt"),
            pair("lzss.bad.invalid-compression-options-in-header.txt", "encoded data is corrupt"),
            pair("lzss.bad.missing-padding-at-end-of-data.txt", "encoded data is corrupt"),
            pair("lzss.bad.reference-at-beginning-of-file.txt", "encoded data is corrupt: reference outside of sliding window"),
            pair("lzss.bad.reference-outside-of-non-empty-sliding-window.txt", "encoded data is corrupt: reference outside of sliding window"));
        INFO(std::format("Test parameters: {}", filename));

        CHECK_THROWS_MATCHES(
            decode_file(decoder, filename),
            agbpack::decode_exception,
            Catch::Matchers::Message(expected_exception_message));

        CHECK_THROWS_MATCHES(
            decode_file_to_random_access_iterator(decoder, filename, *this),
            agbpack::decode_exception,
            Catch::Matchers::Message(expected_exception_message));
    }

    SECTION("VRAM safe decoding is disabled by default")
    {
        CHECK(decoder.vram_safe() == false);
    }

    SECTION("VRAM safe decoding can be enabled and disabled")
    {
        decoder.vram_safe(true);
        CHECK(decoder.vram_safe() == true);

        decoder.vram_safe(false);
        CHECK(decoder.vram_safe() == false);
    }

    SECTION("Decoder throws if it is in VRAM safe mode and data is not VRAM safe")
    {
        const auto not_vram_safe_encoded_data = read_encoded_file("lzss.good.reference-with-maximum-match-length.txt");

        decoder.vram_safe(true);
        CHECK_THROWS_MATCHES(
            decode_vector(decoder, not_vram_safe_encoded_data),
            agbpack::decode_exception,
            Catch::Matchers::Message("encoded data is corrupt: encoded data is not VRAM safe"));
    }
}

}

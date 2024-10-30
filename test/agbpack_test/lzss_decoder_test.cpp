// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <cstddef>
#include <filesystem>
#include <string>
#include "testdata.hpp"

import agbpack;

namespace agbpack_test
{

using string = std::string;

namespace
{

/* // TODO: rework this
std::size_t guess_uncompressed_size(const string& basename)
{

    // No .decoded file. Then try reading uncompressed size from .encoded file.
    auto encoded_file_content = read_file(std::filesystem::path(basename).replace_extension("encoded").string());
    if (encoded_file_content.size() >= 4)
    {
        std::size_t uncompressed_size = encoded_file_content[1];
        uncompressed_size += encoded_file_content[2] * 256u;
        uncompressed_size += encoded_file_content[3] * 256u * 256u;
        return uncompressed_size;
    }

    // Cannot even guess size from header.
    // Assume encoded file is so broken that no output will be created.
    return 0;
}

template <typename TDecoder>
std::vector<unsigned char> decode_file_to_random_access_iterator(TDecoder& decoder, const string& basename)
{
    std::vector<unsigned char> input = read_file(basename);
    std::vector<unsigned char> output(guess_uncompressed_size(basename));
    decoder.decode(input.begin(), input.end(), begin(output));
    return output;
}
*/

std::size_t guess_uncompressed_size(const string& basename, const test_data_fixture& fixture)
{
    // Get size of uncompressed data from .decoded file if it exists.
    auto decoded_file_path = fixture.get_decoded_file_path(basename);
    if (std::filesystem::exists(decoded_file_path))
    {
        return get_file_size(decoded_file_path);
    }

    // TODO: implement
    return 8192;
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
}

// TODO: redo stuff below, as a test fixture thing
/*
TEST_CASE("lzss_decoder_test")
{
    SECTION("Invalid input")
    {
        const auto encoded_file = GENERATE(
            "lzss.bad.eof-inside-header.txt.encoded",
            "lzss.bad.eof-at-flag-byte.txt.encoded",
            "lzss.bad.eof-at-reference-byte-1.txt.encoded",
            "lzss.bad.eof-at-reference-byte-2.txt.encoded",
            "lzss.bad.eof-at-literal.txt.encoded",
            "lzss.bad.reference-goes-past-decompressed-size.txt.encoded",
            "lzss.bad.invalid-compression-type-in-header.txt.encoded",
            "lzss.bad.valid-but-unexpected-compression-type-in-header.txt.encoded",
            "lzss.bad.invalid-compression-options-in-header.txt.encoded",
            "lzss.bad.missing-padding-at-end-of-data.txt.encoded");

        // TODO: get 2nd assertion working again
        CHECK_THROWS_AS(decode_file(decoder, encoded_file), agbpack::decode_exception);
        //CHECK_THROWS_AS(decode_file_to_random_access_iterator(decoder, encoded_file), agbpack::decode_exception);
    }
}*/

}

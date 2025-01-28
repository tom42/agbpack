// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <cstddef>
#include <format>
#include "testdata.hpp"

import agbpack;

namespace agbpack_test
{

using agbpack::lzss_decoder;
using agbpack::lzss_encoder;

namespace
{

class test_parameters final
{
public:
    test_parameters(const char* filename, std::size_t expected_encoded_size)
        : m_filename(filename)
        , m_expected_encoded_size(expected_encoded_size)
    {}

    const char* filename() const { return m_filename; }

    std::size_t expected_encoded_size() const { return m_expected_encoded_size; }

private:
    const char* m_filename;
    std::size_t m_expected_encoded_size;
};

}

TEST_CASE_METHOD(test_data_fixture, "lzss_encoder_test")
{
    lzss_encoder encoder;
    lzss_decoder decoder;
    set_test_data_directory("lzss_encoder");

    SECTION("Successful encoding")
    {
        const auto parameters = GENERATE(
            test_parameters("lzss.good.zero-length-file.txt", 4),
            test_parameters("lzss.good.1-literal-byte.txt", 8),
            test_parameters("lzss.good.3-literal-bytes.txt", 8),
            test_parameters("lzss.good.8-literal-bytes.txt", 16),
            test_parameters("lzss.good.9-literal-bytes.txt", 16),
            test_parameters("lzss.good.minimum-match.txt", 12),
            test_parameters("lzss.good.maximum-match.txt", 28),
            test_parameters("lzss.good.delta.cppm", 1556));
        INFO(std::format("Test parameters: {}", parameters.filename()));
        const auto original_data = read_decoded_file(parameters.filename());

        // Encode
        const auto encoded_data = encode_vector(encoder, original_data);
        CHECK(encoded_data.size() == parameters.expected_encoded_size());

        // Decode
        const auto decoded_data = decode_vector(decoder, encoded_data);
        CHECK(decoded_data == original_data);
    }

    SECTION("VRAM safe encoding is disabled by default")
    {
        CHECK(encoder.vram_safe() == false);
    }

    SECTION("VRAM safe encoding can be enabled and disabled")
    {
        encoder.vram_safe(true);
        CHECK(encoder.vram_safe() == true);

        encoder.vram_safe(false);
        CHECK(encoder.vram_safe() == false);
    }

    SECTION("VRAM safe encoding")
    {
        const std::vector<unsigned char> original_data = { 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a' };

        // Encode for WRAM as reference
        encoder.vram_safe(false);
        CHECK(encode_vector(encoder, original_data).size() == 8);

        // Encode for VRAM, should be bigger
        encoder.vram_safe(true);
        const auto vram_safe_encoded_data = encode_vector(encoder, original_data);
        CHECK(vram_safe_encoded_data.size() == 12);

        // Decode VRAM safe data
        const auto decoded_data = decode_vector(decoder, vram_safe_encoded_data);
        CHECK(decoded_data == original_data);
    }
}

}

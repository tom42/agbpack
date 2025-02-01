// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <cstddef>
#include <format>
#include <tuple>
#include "testdata.hpp"

import agbpack;

namespace agbpack_test
{

using size_t = std::size_t;
using agbpack::lzss_decoder;
using agbpack::lzss_encoder;
using agbpack::optimal_lzss_encoder;

namespace
{

// We have no need for a test fixture, but all
// the template test method macros want to have one.
template <typename TUnused>
class lzss_encoder_test_data_fixture : public test_data_fixture {};

class test_parameters final
{
public:
    test_parameters(const char* filename, size_t expected_greedy_encoded_size, size_t expected_optimal_encoded_size)
        : m_filename(filename)
        , m_expected_greedy_encoded_size(expected_greedy_encoded_size)
        , m_expected_optimal_encoded_size(expected_optimal_encoded_size)
    {}

    const char* filename() const { return m_filename; }

    size_t expected_encoded_size(const lzss_encoder&) const { return m_expected_greedy_encoded_size; }

    size_t expected_encoded_size(const optimal_lzss_encoder&) const { return m_expected_optimal_encoded_size; }

private:
    const char* m_filename;
    size_t m_expected_greedy_encoded_size;
    size_t m_expected_optimal_encoded_size;
};

using lzss_encoder_types = std::tuple<lzss_encoder, optimal_lzss_encoder>;

}

TEMPLATE_LIST_TEST_CASE_METHOD(
    lzss_encoder_test_data_fixture,
    "lzss_encoder_test",
    "[lzss]",
    lzss_encoder_types)
{
    TestType encoder;
    lzss_decoder decoder;
    this->set_test_data_directory("lzss_encoder");

    SECTION("Successful encoding")
    {
        const auto parameters = GENERATE(
            test_parameters("lzss.good.zero-length-file.txt",  4,    4),
            test_parameters("lzss.good.1-literal-byte.txt",    8,    8),
            test_parameters("lzss.good.3-literal-bytes.txt",   8,    8),
            test_parameters("lzss.good.8-literal-bytes.txt",  16,   16),
            test_parameters("lzss.good.9-literal-bytes.txt",  16,   16),
            test_parameters("lzss.good.minimum-match.txt",    12,   12),
            test_parameters("lzss.good.maximum-match.txt",    28,   28),
            test_parameters("lzss.good.delta.cppm",         1556, 1572)); // TODO: bummer: optimal parsing yields larger result than greedy. That's not good at all, is it?
        INFO(std::format("Test parameters: {}", parameters.filename()));
        const auto original_data = this->read_decoded_file(parameters.filename());

        // Encode
        const auto encoded_data = encode_vector(encoder, original_data);
        CHECK(encoded_data.size() == parameters.expected_encoded_size(encoder));

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

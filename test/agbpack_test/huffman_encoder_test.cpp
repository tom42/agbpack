// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <cstddef>
#include <format>
#include <stdexcept>
#include <string>
#include <utility>
#include "testdata.hpp"

import agbpack;

namespace agbpack_test
{

using agbpack::huffman_decoder;
using agbpack::huffman_encoder;
using agbpack::huffman_options;
using std::size_t;
using std::string;

namespace
{

class test_parameters final
{
public:
    explicit test_parameters(const string& filename, size_t expected_encoded_size_h4, size_t expected_encoded_size_h8)
        : m_filename(filename)
        , m_expected_encoded_size_h4(expected_encoded_size_h4)
        , m_expected_encoded_size_h8(expected_encoded_size_h8)
    {}

    const string& filename() const { return m_filename; }

    size_t expected_encoded_size(huffman_options options) const
    {
        switch (options)
        {
            case huffman_options::h4:
                return m_expected_encoded_size_h4;
            case huffman_options::h8:
                return m_expected_encoded_size_h8;
        }

        throw std::invalid_argument("invalid options");
    }

private:
    string m_filename;
    size_t m_expected_encoded_size_h4;
    size_t m_expected_encoded_size_h8;
};

}

TEST_CASE_METHOD(test_data_fixture, "huffman_encoder_test")
{
    huffman_encoder encoder;
    huffman_decoder decoder;
    set_test_data_directory("huffman_encoder");

    SECTION("Construction without options")
    {
        CHECK(encoder.options() == huffman_options::h8);
    }

    SECTION("Construction with valid options")
    {
        const auto options = GENERATE(huffman_options::h4, huffman_options::h8);
        huffman_encoder e(options);
        CHECK(e.options() == options);
    }

    SECTION("Construction with invalid options")
    {
        CHECK_THROWS_MATCHES(
            huffman_encoder(huffman_options(-1)),
            std::invalid_argument,
            Catch::Matchers::Message("TODO: exception message"));
    }

    SECTION("Successful encoding")
    {
        // Note: not too much thought has been put into constructing test data for 4 bit huffman coding,
        // based on the assumption that 8 bit encoding is the hairy bit due to overflow problems in
        // huffman tree serialization. Basically we just encode all the data which is constructed with
        // 8 bit huffman coding in mind also using 4 bit encoding.
        const auto huffman_options = GENERATE(huffman_options::h4, huffman_options::h8);
        const auto parameters = GENERATE(
            test_parameters("huffman.good.8.0-bytes.txt", 8, 8),
            test_parameters("huffman.good.8.1-byte.txt", 12, 12),
            test_parameters("huffman.good.8.2-bytes.txt", 16, 12),
            test_parameters("huffman.good.8.helloworld.txt", 32, 24),
            test_parameters("huffman.good.8.foo.txt", 52, 44),
            test_parameters("huffman.good.8.256-bytes-with-same-frequency.bin", 292, 772));
        INFO(std::format("Test parameters: {}, {} bit encoding", parameters.filename(), std::to_underlying(huffman_options)));
        const auto original_data = read_decoded_file(parameters.filename());

        // Encode
        encoder.options(huffman_options);
        const auto encoded_data = encode_vector(encoder, original_data);
        CHECK(encoded_data.size() == parameters.expected_encoded_size(huffman_options));

        // Decode
        const auto decoded_data = decode_vector(decoder, encoded_data);
        CHECK(decoded_data == original_data);
    }

    SECTION("Invalid options")
    {
        CHECK_THROWS_MATCHES(
            encoder.options(huffman_options(-1)),
            std::invalid_argument,
            Catch::Matchers::Message("invalid huffman compression options"));
    }
}

}

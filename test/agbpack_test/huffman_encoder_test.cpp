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

using string = std::string;

class test_parameters final
{
public:
    explicit test_parameters(const std::string& decoded_file_name, std::size_t expected_encoded_size_h4, std::size_t expected_encoded_size_h8)
        : m_decoded_file_name(decoded_file_name)
        , m_expected_encoded_size_h4(expected_encoded_size_h4)
        , m_expected_encoded_size_h8(expected_encoded_size_h8)
    {}

    const std::string& decoded_file_name() const { return m_decoded_file_name; }

    std::size_t expected_encoded_size(agbpack::huffman_options options) const
    {
        switch (options)
        {
            case agbpack::huffman_options::h4:
                return m_expected_encoded_size_h4;
            case agbpack::huffman_options::h8:
                return m_expected_encoded_size_h8;
            default:
                throw std::invalid_argument("invalid options");
        }
    }

private:
    std::string m_decoded_file_name;
    std::size_t m_expected_encoded_size_h4;
    std::size_t m_expected_encoded_size_h8;
};

TEST_CASE("huffman_encoder_test")
{
    agbpack::huffman_encoder encoder;
    agbpack::huffman_decoder decoder;
    test_data_directory test_data_directory("huffman_encoder");

    SECTION("Successful encoding")
    {
        // TODO: rethink filename pattern
        // TODO: also rewrite all other tests to use codec/test specific subdirectories
        //       * In the case of the huffman decoder test we should check which files are still
        //         needed in the decoder directory and wich are used by encoder tests only and
        //         do not need to be in the decoder directory
        // TODO: add more tests
        //       * 1 byte (err...what? 1 byte - 1 symbol?
        //       * 2 bytes (err...what? 2 bytes - 2 symbols, or 2 bytes, 2 times same symbol?)
        // TODO: maximum depth huffman code (some sort of lucas sequence thing)
        //       * Can we even reach 31/32 bits?

        // Note: not too much thought has been put into constructing test data for 4 bit huffman coding,
        // based on the assumption that 8 bit encoding is the hairy bit due to overflow problems in
        // huffman tree serialization. Basically we just encode all the data which is constructed with
        // 8 bit huffman coding in mind also using 4 bit encoding.
        const auto huffman_options = GENERATE(agbpack::huffman_options::h4, agbpack::huffman_options::h8);
        const auto parameters = GENERATE(
            test_parameters("huffman.good.8.0-bytes.txt", 8, 8),
            test_parameters("huffman.good.8.helloworld.txt", 32, 24),
            test_parameters("huffman.good.8.foo.txt", 52, 44),
            // TODO: this file needs a better name.
            //       * The fun here is, we have 256 symbols with all the same frequency. Incidentally we fail at encoding it
            test_parameters("huffman.good.8.256-bytes.bin", 292, 772));
        INFO(std::format("Test parameters: {}, {} bit encoding", parameters.decoded_file_name(), std::to_underlying(huffman_options)));
        const auto original_data = test_data_directory.read_decoded_file(parameters.decoded_file_name());

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
            encoder.options(agbpack::huffman_options(-1)),
            std::invalid_argument,
            Catch::Matchers::Message("invalid huffman compression options"));
    }
}

// TODO: redo all crap below
// TODO: add test: input data too big
// TODO: add test: 256 symbols with same frequency (do we need a 4 bit test with 16 symbols? maybe, but we can use the 256 file in principle, no?)
// TODO: add test: deep code constructed with lucas sequence (4 bit, 8 bit is not really possible, no?)
TEST_CASE("huffman_encoder_test_old")
{
    agbpack::huffman_encoder encoder;

    SECTION("Successful 4 bit encoding with reference encoded data")
    {
        // TODO: add necessary tests (see 8 bit counterpart)
        const string filename_part = GENERATE(
            "huffman.good.4.0-bytes.txt");
        const auto expected_data = read_file(filename_part + ".encoded");

        encoder.options(agbpack::huffman_options::h4);
        const auto encoded_data = encode_file(encoder, filename_part + ".decoded");

        CHECK(encoded_data == expected_data);
    }

    SECTION("Successful 8 bit encoding with check against decoder")
    {
        // TODO: have dedicated test input that requires padding of the bitstream (huffman.good.frequency-table-test.txt.decoded does this, but it has a bad name)
        // TODO: have dedicated test input that requires flushing of the bitstream (huffman.good.frequency-table-test.txt.decoded does this too, but it really has a bad name)
        const string filename = GENERATE("huffman.good.frequency-table-test.txt.decoded");
        const auto original_data = read_file(filename);

        // Encode
        encoder.options(agbpack::huffman_options::h8);
        const auto encoded_data = encode_vector(encoder, original_data);

        // Decode
        agbpack::huffman_decoder decoder;
        const auto decoded_data = decode_vector(decoder, encoded_data);

        CHECK(decoded_data == original_data);
    }

    SECTION("Successful 4 bit encoding with check against decoder")
    {
        const string filename = GENERATE(
            "huffman.good.4.0-bytes.txt.decoded",
            "huffman.good.frequency-table-test.txt.decoded");
        const auto original_data = read_file(filename);

        // Encode
        encoder.options(agbpack::huffman_options::h4);
        const auto encoded_data = encode_vector(encoder, original_data);

        // Decode
        agbpack::huffman_decoder decoder;
        const auto decoded_data = decode_vector(decoder, encoded_data);

        CHECK(decoded_data == original_data);
    }
}

}

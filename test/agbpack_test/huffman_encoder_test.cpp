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
using size_t = std::size_t;

class test_parameters final
{
public:
    explicit test_parameters(const string& filename, size_t expected_encoded_size_h4, size_t expected_encoded_size_h8)
        : m_filename(filename)
        , m_expected_encoded_size_h4(expected_encoded_size_h4)
        , m_expected_encoded_size_h8(expected_encoded_size_h8)
    {}

    const string& filename() const { return m_filename; }

    size_t expected_encoded_size(agbpack::huffman_options options) const
    {
        switch (options)
        {
            case agbpack::huffman_options::h4:
                return m_expected_encoded_size_h4;
            case agbpack::huffman_options::h8:
                return m_expected_encoded_size_h8;
        }

        throw std::invalid_argument("invalid options");
    }

private:
    string m_filename;
    size_t m_expected_encoded_size_h4;
    size_t m_expected_encoded_size_h8;
};

TEST_CASE("huffman_encoder_test")
{
    agbpack::huffman_encoder encoder;
    agbpack::huffman_decoder decoder;
    test_data_directory test_data_directory("huffman_encoder");

    SECTION("Successful encoding")
    {
        // TODO: rethink filename pattern
        // TODO: have dedicated test input that requires padding of the bitstream (huffman.good.frequency-table-test.txt.decoded does this, but it has a bad name)
        // TODO: have dedicated test input that requires flushing of the bitstream (huffman.good.frequency-table-test.txt.decoded does this too, but it really has a bad name)
        // TODO: also rewrite all other tests to use codec/test specific subdirectories
        //       * In the case of the huffman decoder test we should check which files are still
        //         needed in the decoder directory and wich are used by encoder tests only and
        //         do not need to be in the decoder directory
        // TODO: add more tests
        //       * 1 byte (err...what? 1 byte - 1 symbol?
        //       * 2 bytes (err...what? 2 bytes - 2 symbols, or 2 bytes, 2 times same symbol?)
        // TODO: maximum depth huffman code (some sort of lucas sequence thing)
        //       * Can we even reach 31/32 bits? Let alone 255 bits?
        //         * 255/256 bits is rather impossible due to storage requirements
        //         * 31/32 bits I don't know, but it's what our decoder supports currently at most
        //           * We could do it differently: given the maximum file uncompressed size is 2^24-1 byte (or is it?)
        //           * What is the maximum lucas sequence thing that would fit into such a file?
        //             * We could generate that file rather than storing it on disk, no?
        //       * Try at least 16 bits, which is the maximum for 4 bit encoding, and which, incidentally, is what we currently care more for anyway
        // TODO: add test: input data too big

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
            //       * Also verify it really has each symbol exactly once?
            test_parameters("huffman.good.8.256-bytes.bin", 292, 772));
        INFO(std::format("Test parameters: {}, {} bit encoding", parameters.filename(), std::to_underlying(huffman_options)));
        const auto original_data = test_data_directory.read_decoded_file(parameters.filename());

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

}

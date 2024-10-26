// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <cstddef>
#include <stdexcept>
#include <string>
#include "testdata.hpp"

import agbpack;

namespace agbpack_test
{

using string = std::string;

struct test_parameters
{
    std::string decoded_file_name;
    std::size_t expected_encoded_size;
};

TEST_CASE("huffman_encoder_test")
{
    agbpack::huffman_encoder encoder;
    agbpack::huffman_decoder decoder;
    test_data_directory test_data_directory("huffman_encoder");

    SECTION("Successful 8 bit encoding")
    {
        // TODO: rethink filename pattern
        // TODO: add more tests
        //       * 1 byte (err...what? 1 byte - 1 symbol?
        //       * 2 bytes (err...what? 2 bytes - 2 symbols, or 2 bytes, 2 times same symbol?)
        const auto parameters = GENERATE(
            test_parameters("huffman.good.8.0-bytes.txt", 8));
        INFO("Test file: " + parameters.decoded_file_name);
        const auto original_data = test_data_directory.read_decoded_file(parameters.decoded_file_name);

        // Encode
        encoder.options(agbpack::huffman_options::h8);
        const auto encoded_data = encode_vector(encoder, original_data);
        REQUIRE(encoded_data.size() == parameters.expected_encoded_size);

        // Decode
        const auto decoded_data = decode_vector(decoder, encoded_data);
        CHECK(decoded_data == original_data);
    }

}

// TODO: redo all crap below
// TODO: add test: input data too big
// TODO: add test: 256 symbols with same frequency (do we need a 4 bit test with 16 symbols? maybe, but we can use the 256 file in principle, no?)
// TODO: add test: deep code constructed with lucas sequence (4 bit, 8 bit is not really possible, no?)
TEST_CASE("huffman_encoder_test_old")
{
    agbpack::huffman_encoder encoder;

    SECTION("Successful 8 bit encoding with reference encoded data")
    {
        // TODO: add necessary tests:
        //       - 1 byte
        //       - 2 byte
        const string filename_part = GENERATE(
            "huffman.good.8.0-bytes.txt");
        const auto expected_data = read_file(filename_part + ".encoded");

        encoder.options(agbpack::huffman_options::h8);
        const auto encoded_data = encode_file(encoder, filename_part + ".decoded");

        CHECK(encoded_data == expected_data);
    }

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
        const string filename = GENERATE(
            "huffman.good.8.0-bytes.txt.decoded",
            "huffman.good.frequency-table-test.txt.decoded");
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

    SECTION("Invalid options")
    {
        CHECK_THROWS_MATCHES(
            encoder.options(agbpack::huffman_options(-1)),
            std::invalid_argument,
            Catch::Matchers::Message("invalid huffman compression options"));
    }
}

// TODO: for development/debugging, remove
TEST_CASE("zzz_test")
{
    // TODO: Get this test running:
    //       * In the production code, hardcode a tree we obtained from CUE in some way
    //       * Copy that tree to the compressed output
    //       * Use that tree to encode the output
    //       * Do this both for
    //         * 8 bit
    //         * 4 bit
    //         * This gives us an encoder that is complete except for tree serialization, which we can then tackle last
    // TODO: Implement test
    //       * get some test data
    //         * Foo?
    //         * Some sort of lucas sequence number thing? => Maybe the latter
    //         * Another test: equally distributed symbols with 8 and 4 bit encoding
    //       * Create reference encoded data
    //       * Dump codes
    //       * Create codes using our tree building mechanism
    //       * Does it print the same output?
    // TODO: see whether we have any other use for the following files (e.g. use them in some test)
    //       * huffman.good.8.foo.txt
    //       * huffman.good.8.helloworld.txt
    //
    const string basename = "huffman.good.8.helloworld.txt";
    const auto original_data = read_file(basename + ".decoded");

    // Encode data
    agbpack::huffman_encoder encoder;
    encoder.options(agbpack::huffman_options::h8);
    const auto encoded_data = encode_vector(encoder, original_data);

    // Decode data: check whether our decoder can read the data produced by our encoder
    agbpack::huffman_decoder decoder;
    const auto decoded_data = decode_vector(decoder, encoded_data);
    CHECK(decoded_data == original_data);
}

}

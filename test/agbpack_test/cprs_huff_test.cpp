// TODO: Delete this file once done

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <string>
#include <utility>
#include "testdata.hpp"

import agbpack;

namespace agbpack_test
{

using agbpack::huffEncode;
using std::string;

class test_parameters123 final
{
public:
    explicit test_parameters123(const string& filename, size_t expected_encoded_size_h4, size_t expected_encoded_size_h8)
        : m_filename(filename)
        , m_expected_encoded_size_h4(expected_encoded_size_h4)
        , m_expected_encoded_size_h8(expected_encoded_size_h8)
    {
    }

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

// TODO: implement all cases from
//       * huffman_tree_serializer_test
TEST_CASE_METHOD(test_data_fixture, "cprs_huff_test")
{
    agbpack::huffman_decoder decoder;
    set_test_data_directory("huffman_encoder");

    const auto huffman_options = GENERATE(agbpack::huffman_options::h4, agbpack::huffman_options::h8);
    const auto parameters = GENERATE(
        //test_parameters123("huffman.good.8.0-bytes.txt", 8, 8), // TODO: get this to work (segfaults for some inane reason)
        test_parameters123("huffman.good.8.helloworld.txt", 32, 24),
        test_parameters123("huffman.good.8.foo.txt", 52, 44),
        test_parameters123("huffman.good.8.256-bytes.bin", 292, 772));
    INFO(std::format("Test parameters: {}, {} bit encoding", parameters.filename(), std::to_underlying(huffman_options)));
    const auto original_data = read_decoded_file(parameters.filename());

    // Encode
    const auto encoded_data = huffEncode(original_data.data(), original_data.size(), huffman_options == agbpack::huffman_options::h4);
    CHECK(encoded_data.size() == parameters.expected_encoded_size(huffman_options));

    // Decode
    const auto decoded_data = decode_vector(decoder, encoded_data);
    CHECK(decoded_data == original_data);
}


}

// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#ifndef AGBPACK_TESTDATA_HPP_20240706
#define AGBPACK_TESTDATA_HPP_20240706

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

// TODO: see which of the free functions in here we still need
namespace agbpack_test
{

std::size_t get_file_size(const std::string& path);

std::ifstream open_binary_file(const std::string& path);

std::string get_testfile_path(const std::string& basename);

std::vector<unsigned char> read_file(const std::string& basename);

template <typename TDecoder>
std::vector<unsigned char> decode_vector(TDecoder& decoder, const std::vector<unsigned char>& input)
{
    std::vector<unsigned char> output;
    decoder.decode(input.begin(), input.end(), back_inserter(output));
    return output;
}

template <typename TDecoder>
std::vector<unsigned char> decode_file(TDecoder& decoder, const std::string& basename)
{
    return decode_vector(decoder, read_file(basename));
}

template <typename TEncoder>
std::vector<unsigned char> encode_vector(TEncoder& encoder, const std::vector<unsigned char>& input)
{
    std::vector<unsigned char> output;
    encoder.encode(input.begin(), input.end(), back_inserter(output));
    return output;
}

template <typename TEncoder>
std::vector<unsigned char> encode_file(TEncoder& encoder, const std::string& basename)
{
    return encode_vector(encoder, read_file(basename));
}

// TODO: if this works out, rewrite all tests to use this class and sort test files into subdirectories
class test_data_directory final
{
public:
    explicit test_data_directory(const std::string& directory) : m_directory(directory) {}

    std::vector<unsigned char> read_decoded_file(const std::string& basename);

private:
    std::string m_directory;
};

}

#endif

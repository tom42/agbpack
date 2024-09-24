// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#ifndef AGBPACK_TESTDATA_HPP_20240706
#define AGBPACK_TESTDATA_HPP_20240706

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

namespace agbpack_test
{

std::size_t get_file_size(const std::string& path);

std::ifstream open_binary_file(const std::string& path);

std::string get_testfile_path(const std::string& basename);

const std::vector<unsigned char> read_file(const std::string& basename);

template <typename TDecoder>
const std::vector<unsigned char> decode_file(TDecoder& decoder, const std::string& basename)
{
    std::vector<unsigned char> input = read_file(basename);
    std::vector<unsigned char> output;
    decoder.decode(input.begin(), input.end(), back_inserter(output));
    return output;
}

template <typename TEncoder>
const std::vector<unsigned char> encode_vector(TEncoder& encoder, const std::vector<unsigned char>& input)
{
    std::vector<unsigned char> output;
    encoder.encode(input.begin(), input.end(), back_inserter(output));
    return output;
}

template <typename TEncoder>
const std::vector<unsigned char> encode_file(TEncoder& encoder, const std::string& basename)
{
    std::vector<unsigned char> input = read_file(basename);
    std::vector<unsigned char> output;
    encoder.encode(input.begin(), input.end(), back_inserter(output));
    return output;
}

}

#endif

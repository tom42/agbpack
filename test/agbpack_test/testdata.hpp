// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#ifndef AGBPACK_TESTDATA_HPP_20240706
#define AGBPACK_TESTDATA_HPP_20240706

#include <fstream>
#include <string>
#include <vector>

namespace agbpack_test
{

std::ifstream open_binary_file(const std::string& name);

std::string get_testfile_path(const std::string& basename);

std::vector<unsigned char> read_file(const std::string& basename);

template <typename TDecoder>
std::vector<unsigned char> decode_file(TDecoder& decoder, const std::string& basename)
{
    std::vector<unsigned char> input = read_file(basename);
    std::vector<unsigned char> output;
    decoder.decode(input.begin(), input.end(), back_inserter(output));
    return output;
}

}

#endif

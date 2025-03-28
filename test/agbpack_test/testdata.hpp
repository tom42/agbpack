// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#ifndef AGBPACK_TESTDATA_HPP_20240706
#define AGBPACK_TESTDATA_HPP_20240706

#include <fstream>
#include <string>
#include <vector>

namespace agbpack_test
{

std::size_t get_file_size(const std::string& path);

std::ifstream open_binary_file(const std::string& path);

template <typename TDecoder>
std::vector<unsigned char> decode_vector(TDecoder& decoder, const std::vector<unsigned char>& input)
{
    std::vector<unsigned char> output;
    decoder.decode(input.begin(), input.end(), back_inserter(output));
    return output;
}

template <typename TEncoder>
std::vector<unsigned char> encode_vector(TEncoder& encoder, const std::vector<unsigned char>& input)
{
    std::vector<unsigned char> output;
    encoder.encode(input.begin(), input.end(), back_inserter(output));
    return output;
}

class test_data_directory final
{
public:
    explicit test_data_directory(const std::string& directory) : m_directory(directory) {}

    std::string get_decoded_file_path(const std::string& basename) const;

    std::string get_encoded_file_path(const std::string& basename) const;

private:
    std::string m_directory;
};

class test_data_fixture
{
public:
    void set_test_data_directory(const std::string& directory);

    std::string get_decoded_file_path(const std::string& basename) const;

    std::string get_encoded_file_path(const std::string& basename) const;

    std::vector<unsigned char> read_decoded_file(const std::string& basename) const;

    std::vector<unsigned char> read_encoded_file(const std::string& basename) const;

    template <typename TDecoder>
    std::vector<unsigned char> decode_file(TDecoder& decoder, const std::string& basename) const
    {
        return decode_vector(decoder, read_encoded_file(basename));
    }

    template <typename TEncoder>
    std::vector<unsigned char> encode_file(TEncoder& encoder, const std::string& basename) const
    {
        return encode_vector(encoder, read_decoded_file(basename));
    }

private:
    test_data_directory m_directory = test_data_directory("");
};

}

#endif

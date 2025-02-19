// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <fstream>

module agbpack_test_tools_common;

namespace agbpack_test_tools_common
{

ifstream open_binary_input_file(const string& path)
{
    ifstream file;

    file.exceptions(ifstream::badbit | ifstream::eofbit | ifstream::failbit);
    file.open(path, std::ios_base::binary);

    // Set badbit only for processing.
    // Caution: I have no clue what I am doing here.
    file.exceptions(ifstream::badbit);
    file.unsetf(std::ios::skipws); // Required to correctly read binary files using some APIs, e.g. std::istream_iterator.
    return file;
}

ofstream open_binary_output_file(const string& path)
{
    ofstream file;

    file.exceptions(ifstream::badbit | ifstream::eofbit | ifstream::failbit);
    file.open(path, std::ios_base::binary);

    // Set badbit only for processing.
    // Caution: I have no clue what I am doing here.
    file.exceptions(ifstream::badbit);
    file.unsetf(std::ios::skipws); // Required to correctly read binary files using some APIs, e.g. std::istream_iterator.
    return file;
}

}

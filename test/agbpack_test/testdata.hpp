// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#ifndef AGBPACK_TESTDATA_HPP_20240706
#define AGBPACK_TESTDATA_HPP_20240706

#include <string>
#include <vector>

namespace agbpack_test
{

std::string get_testfile_path(const std::string& basename);

std::vector<unsigned char> read_testfile(const std::string& basename);

}

#endif

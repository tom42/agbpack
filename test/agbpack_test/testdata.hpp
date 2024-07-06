// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#ifndef AGBPACK_TESTDATA_HPP_20240706
#define AGBPACK_TESTDATA_HPP_20240706

#include <string>
#include <vector>

namespace agbpack_test
{

std::vector<unsigned char> read_testdata_file(const std::string& basename);

}

#endif

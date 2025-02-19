// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <fstream>
#include <string>

export module agbpack_test_tools_common;

namespace agbpack_test_tools_common
{

export std::ifstream open_binary_input_file(const std::string& path);
export std::ofstream open_binary_output_file(const std::string& path);

}

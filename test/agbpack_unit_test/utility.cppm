// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <vector>

export module agbpack_unit_test.utility;
import agbpack;

namespace agbpack_unit_test
{

export std::vector<agbpack::symbol_frequency> lucas_sequence(std::size_t length);

}

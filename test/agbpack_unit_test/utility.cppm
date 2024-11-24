// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <vector>

export module agbpack_unit_test.utility;
import agbpack;

namespace agbpack_unit_test
{

// Generate Lucas sequence.
// If such a sequence of length N is used as symbol frequencies, then the resulting huffman tree's depth is N-1.
// See https://stackoverflow.com/a/66165482/17365470
export std::vector<agbpack::symbol_frequency> lucas_sequence(std::size_t length);

}

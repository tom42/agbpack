// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <vector>
#include <stdexcept>

export module agbpack_unittest.utility;
import agbpack;

namespace agbpack_unittest
{

using agbpack::symbol_frequency;

// TODO: put this into an implementation file? (must remove inline then!)
inline std::vector<symbol_frequency> lucas_sequence(size_t length)
{
    std::vector<symbol_frequency> sequence{1, 1, 1, 3};

    if (length < sequence.size())
    {
        throw std::out_of_range("sequence length too short");
    }

    for (size_t i = sequence.size(); i < length; ++i)
    {
        sequence.push_back(sequence[i - 1] + sequence[i - 2]);
    }

    return sequence;
}

}

// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <vector>
#include <stdexcept>

module agbpack_unit_test.utility;
import agbpack;

namespace agbpack_unit_test
{

std::vector<agbpack::symbol_frequency> lucas_sequence(std::size_t length)
{
    std::vector<agbpack::symbol_frequency> sequence;

    sequence.reserve(length);
    sequence = {1, 1, 1, 3};

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

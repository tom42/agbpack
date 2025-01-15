// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <catch2/catch_test_macros.hpp>
#include <vector>

export module agbpack_unit_test;
import agbpack;

export namespace Catch
{

template <>
struct StringMaker<agbpack::code_table_entry>
{
    static std::string convert(const agbpack::code_table_entry& e)
    {
        return std::format("(s={}, c={}, l={})", e.s(), e.c(), e.l());
    }
};

// TODO: are we risking ODR violations here?
//       See https://brevzin.github.io/c++/2023/01/19/debug-fmt-catch/
// TODO: need to test this with visual studio
// TODO: include this in ALL translation units (should we have a separate test helper module and unit test assembly?)
// TODO: install README that clearly states that ALL translation units should import this to avoid ODR violations
template <>
struct StringMaker<agbpack::match>
{
    static std::string convert(const agbpack::match& m)
    {
        return std::format("(length={}, offset={})", m.length(), m.offset());
    }
};

}

namespace agbpack_unit_test
{

// Generate a modified Lucas sequence as described here: https://stackoverflow.com/a/66165482/17365470
// If the numbers of such a sequence of length N are used as symbol frequencies,
// then the resulting huffman tree's depth is N-1.
export std::vector<agbpack::symbol_frequency> lucas_sequence(std::size_t length);

export agbpack::huffman_encoder_tree create_tree_from_lucas_sequence(std::size_t sequence_length);

}

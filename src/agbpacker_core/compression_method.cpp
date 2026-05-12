// SPDX-FileCopyrightText: 2026 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <array>

module agbpacker_core;

namespace agbpacker_core
{

// TODO: Accessor method, or do we simply declare it publicly?
constexpr std::array<compression_method_info, 1> compression_methods =
{
	// TODO: add all here. How can we have constexpr strings in there? is that even possible?
	compression_method_info(),
};

}

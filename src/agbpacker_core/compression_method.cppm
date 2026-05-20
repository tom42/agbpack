// SPDX-FileCopyrightText: 2026 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <span>
#include <string>

export module agbpacker_core:compression_method;

namespace agbpacker_core
{

AGBPACK_EXPORT_FOR_UNIT_TESTING
enum class compression_method
{
    lzss,
    optimal_lzss,
    h4,
    h8,
    rle,
    d8,
    d16
};

struct compression_method_info final
{
    compression_method method;
    const char* name;
};

std::span<const compression_method_info> all_compression_methods();

}

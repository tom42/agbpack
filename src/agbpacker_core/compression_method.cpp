// SPDX-FileCopyrightText: 2026 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <array>
#include <span>

module agbpacker_core;

namespace agbpacker_core
{

namespace
{

constexpr std::array<compression_method_info, 7> compression_methods =
{
    compression_method_info{ compression_method::lzss, "lzss" },
    compression_method_info{ compression_method::optimal_lzss, "optimal_lzss" },
    compression_method_info{ compression_method::h4, "h4" },
    compression_method_info{ compression_method::h8, "h8" },
    compression_method_info{ compression_method::rle, "rle" },
    compression_method_info{ compression_method::d8, "d8" },
    compression_method_info{ compression_method::d16, "d16" }
};

}

std::span<const compression_method_info> all_compression_methods()
{
    return compression_methods;
}

}

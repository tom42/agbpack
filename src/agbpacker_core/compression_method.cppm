// SPDX-FileCopyrightText: 2026 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

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
    delta8,
    delta16
};

// TODO: work on this
//       * create compression_method.cpp
//       * add an array with all infos in there
//       * find a way to get at that array, somehow
//       * then, get all compression methods and dump these into command line docs
//       * then, use this to verify compression methods, and integrate that into command line parsing too
struct compression_method_info final
{
    std::string name;
};

}

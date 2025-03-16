// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <utility>

export module argpppp:pf;

namespace argpppp
{

export enum class pf
{
    none = 0,
    parse_argv0 = 0x01,
    no_errs = 0x02,
    no_args = 0x04,
    in_order = 0x08,
    no_help = 0x10,
    no_exit = 0x20,
    long_only = 0x40,
    silent = no_exit | no_errs | no_help
};

export constexpr pf operator|(pf l, pf r)
{
    return pf(std::to_underlying(l) | std::to_underlying(r));
}

ARGPPPP_EXPORT_FOR_UNIT_TESTING
constexpr unsigned int to_uint(pf flags)
{
    return static_cast<unsigned int>(flags);
}

}

// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <utility>

export module argpppp:of;

namespace argpppp
{

// TODO: provide conversion to int => call site? (nah do a to_int(od) function and unit test that)
export enum class of
{
    none = 0,
    arg_optional = 0x1,
    hidden = 0x2,
    alias = 0x4,
    doc = 0x8,
    no_usage = 0x10
};

export constexpr of operator|(of l, of r)
{
    return of(std::to_underlying(l) | std::to_underlying(r));
}

}

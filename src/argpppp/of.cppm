// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

export module argpppp:of;

namespace argpppp
{

// TODO: provide bitwise or for these, and probably also and => needs test (what would we need and for?)
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

}

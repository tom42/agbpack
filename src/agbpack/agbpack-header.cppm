// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

export module agbpack:header;

namespace agbpack
{

enum class compression_type
{
    rle = 3,
    delta = 8
};

enum class delta_options
{
    delta8 = 1,
    delta16 = 2
};

}

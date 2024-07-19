// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module agbpack;

namespace agbpack
{

std::optional<header> header::parse_for_type(compression_type wanted_type, uint32_t header_data)
{
    auto header = parse(header_data);
    if (!header || (header->type() != wanted_type))
    {
        return {};
    }

    return header;
}

}

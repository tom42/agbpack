// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module agbpack;

namespace
{

}

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

std::optional<header> header::parse(uint32_t header_data)
{
    auto type = static_cast<compression_type>((header_data >> 4) & 0xf);
    if (!is_valid(type))
    {
        return {};
    }

    auto options = create_options(type, header_data & 0xf);
    if (!options || !is_valid(*options))
    {
        return {};
    }

    return header(type, *options, (header_data >> 8) & 0xffffff);
}


}

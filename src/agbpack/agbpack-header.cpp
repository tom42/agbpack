// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <cassert>
#include <cstdint>
#include <optional>
#include <variant>

module agbpack;

namespace agbpack
{

namespace
{

bool is_valid(compression_type type)
{
    switch (type)
    {
        case compression_type::lzss:
        case compression_type::huffman:
        case compression_type::rle:
        case compression_type::delta:
            return true;
    }

    return false;
}

bool is_valid(lzss_options options)
{
    return options == lzss_options::reserved;
}

bool is_valid(huffman_options options)
{
    switch (options)
    {
        case huffman_options::h4:
        case huffman_options::h8:
            return true;
    }

    return false;
}

bool is_valid(rle_options options)
{
    return options == rle_options::reserved;
}

bool is_valid(delta_options options)
{
    switch (options)
    {
        case delta_options::delta8:
        case delta_options::delta16:
            return true;
    }

    return false;
}

bool is_valid_variant(compression_options options)
{
    return std::visit([](auto&& opts) { return is_valid(opts); }, options);
}

std::optional<compression_options> create_unvalidated_options(compression_type type, uint32_t options)
{
    switch (type)
    {
        case compression_type::lzss:
            return lzss_options(options);
        case compression_type::huffman:
            return huffman_options(options);
        case compression_type::rle:
            return rle_options(options);
        case compression_type::delta:
            return delta_options(options);
    }

    assert(false && "Unknown compression type passed to create_unvalidated_options");
    return {};
}

}

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
    auto type = compression_type((header_data >> 4) & 0xf);
    if (!is_valid(type))
    {
        return {};
    }

    auto options = create_unvalidated_options(type, header_data & 0xf);
    if (!options || !is_valid_variant(*options))
    {
        return {};
    }

    return header(type, *options, (header_data >> 8) & 0xffffff);
}


}

// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
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

bool is_valid(rle_options options)
{
    return options == rle_options::reserved;
}

bool is_valid_variant(compression_options options)
{
    return std::visit([](auto&& opts) noexcept { return is_valid(opts); }, options);
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

header header::create(huffman_options options, std::size_t uncompressed_size)
{
    return header(compression_type::huffman, options, uncompressed_size);
}

header header::create(rle_options options, std::size_t uncompressed_size)
{
    return header(compression_type::rle, options, uncompressed_size);
}

header header::create(delta_options options, std::size_t uncompressed_size)
{
    return header(compression_type::delta, options, uncompressed_size);
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

header::header(compression_type type, compression_options options, std::size_t uncompressed_size)
{
    if (!is_valid(type))
    {
        throw std::invalid_argument("invalid compression type");
    }

    if (!is_valid_variant(options))
    {
        throw std::invalid_argument("invalid compression options");
    }

    if (uncompressed_size > maximum_uncompressed_size)
    {
        throw encode_exception("data to encode is too big");
    }

    m_type = type;
    m_options = options;
    m_uncompressed_size = static_cast<uint32_t>(uncompressed_size);
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

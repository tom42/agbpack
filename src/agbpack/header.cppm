// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <variant>

export module agbpack:header;

namespace agbpack
{

AGBPACK_EXPORT_FOR_UNIT_TESTING
inline constexpr uint32_t maximum_uncompressed_size = 0xffffff;

export enum class compression_type : unsigned int
{
    lzss = 1,
    huffman = 2,
    rle = 3,
    delta = 8
};

enum class lzss_options : unsigned int
{
    reserved = 0
};

export enum class huffman_options : unsigned int
{
    h4 = 4,
    h8 = 8
};

enum class rle_options : unsigned int
{
    reserved = 0
};

export enum class delta_options : unsigned int
{
    delta8 = 1,
    delta16 = 2
};

inline bool is_valid(delta_options options)
{
    switch (options)
    {
        case delta_options::delta8:
        case delta_options::delta16:
            return true;
    }

    return false;
}

inline bool is_valid(huffman_options options)
{
    switch (options)
    {
        case huffman_options::h4:
        case huffman_options::h8:
            return true;
    }

    return false;
}

using compression_options = std::variant<lzss_options, huffman_options, rle_options, delta_options>;

inline auto as_integer(compression_options options)
{
    return std::visit([](auto&& opts){ return std::to_underlying(opts); }, options);
}

// TODO: rename once the other is gone
export class header2 final
{
public:
    static header2 create_lzss(std::size_t uncompressed_size);
    static header2 create_huffman(huffman_options options, std::size_t uncompressed_size);
    static header2 create_rle(std::size_t uncompressed_size);
    static header2 create_delta(delta_options options, std::size_t uncompressed_size);

    compression_type type() const { return m_type; }

    template <typename TOptions>
    TOptions options() const { return std::get<TOptions>(m_options); }

    uint32_t uncompressed_size() const { return m_uncompressed_size; }

    uint32_t to_uint32_t() const
    {
        return (uncompressed_size() << 8) | (std::to_underlying(type()) << 4) | as_integer(m_options);
    }

    static std::expected<header2, const char*> try_parse(std::span<const unsigned char> s);

    static std::expected<header2, const char*> try_parse(uint32_t header_data);

private:
    compression_type m_type;
    compression_options m_options;
    uint32_t m_uncompressed_size;

    header2(compression_type type, compression_options options, std::size_t uncompressed_size);
};

}

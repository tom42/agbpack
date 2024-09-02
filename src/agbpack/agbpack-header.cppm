// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <cstdint>
#include <optional>
#include <utility>
#include <variant>

export module agbpack:header;

namespace agbpack
{

enum class compression_type : unsigned int
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

enum class huffman_options : unsigned int
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

using compression_options = std::variant<lzss_options, huffman_options, rle_options, delta_options>;

class header final
{
public:
    compression_type type() const { return m_type; }

    compression_options options() const { return m_options; }

    template <typename TOptions>
    TOptions options_as() const
    {
        return std::get<TOptions>(options());
    }

    uint32_t uncompressed_size() const { return m_uncompressed_size; }

    uint32_t to_uint32_t() const
    {
        // TODO: review this, heavily. Particularly not sure about the uncompressed_size thing.
        auto options_as_integer = std::visit([](auto&& opts) { return std::to_underlying(opts); }, options());
        return (uncompressed_size() << 8) | (std::to_underlying(type()) << 4) | options_as_integer;
    }

    static header create(compression_type type, compression_options options, uint32_t uncompressed_size);

    static std::optional<header> parse_for_type(compression_type wanted_type, uint32_t header_data);

private:
    compression_type m_type;
    compression_options m_options;
    uint32_t m_uncompressed_size;

    explicit header(compression_type type, compression_options options, uint32_t uncompressed_size)
        : m_type(type)
        , m_options(options)
        , m_uncompressed_size(uncompressed_size)
    {}

    static std::optional<header> parse(uint32_t header_data);
};

}

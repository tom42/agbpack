// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <cstdint>
#include <optional>
#include <variant>

export module agbpack:header;

namespace agbpack
{

enum class compression_type
{
    lzss = 1,
    huffman = 2,
    rle = 3,
    delta = 8
};

enum class lzss_options
{
    reserved = 0
};

enum class huffman_options
{
    h4 = 4,
    h8 = 8
};

enum class rle_options
{
    reserved = 0
};

enum class delta_options
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

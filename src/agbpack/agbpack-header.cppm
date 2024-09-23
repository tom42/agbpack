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

inline constexpr uint32_t maximum_uncompressed_size = 0xffffff;

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

// TODO: this is a hack: originally I did not want this to be here
//       * Do we want this to be available from library users
//       * Should this be inline (well for the time being yes, because it's in a module interface unit, says clang++)
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

using compression_options = std::variant<lzss_options, huffman_options, rle_options, delta_options>;

inline auto as_integer(compression_options options)
{
    return std::visit([](auto&& opts) noexcept { return std::to_underlying(opts); }, options);
}

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
        // TODO: review/test: is the uncompressed size correctly serialized?
        return (uncompressed_size() << 8) | (std::to_underlying(type()) << 4) | as_integer(options());
    }

    static header create(delta_options options, uint32_t uncompressed_size);

    static std::optional<header> parse_for_type(compression_type wanted_type, uint32_t header_data);

private:
    compression_type m_type;
    compression_options m_options;
    uint32_t m_uncompressed_size;

    // Note: this constructor does not check whether type and options match.
    // That is, you can for instance construct a header with compression type LZSS and RLE compression options.
    // It should therefore not be exposed to the public and remain private.
    explicit header(compression_type type, compression_options options, uint32_t uncompressed_size);

    static std::optional<header> parse(uint32_t header_data);
};

}

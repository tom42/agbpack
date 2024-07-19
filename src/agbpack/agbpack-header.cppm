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
    rle = 3,
    delta = 8
};

enum class lzss_options
{
    reserved = 0
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

using compression_options = std::variant<lzss_options, rle_options, delta_options>;

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
    explicit header(compression_type type, compression_options options, uint32_t uncompressed_size)
        : m_type(type)
        , m_options(options)
        , m_uncompressed_size(uncompressed_size)
    {}

    compression_type m_type;
    compression_options m_options;
    uint32_t m_uncompressed_size;

    static std::optional<header> parse(uint32_t header_data)
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

    static std::optional<compression_options> create_options(compression_type type, uint32_t options)
    {
        switch (type)
        {
            case compression_type::lzss:
                return lzss_options(options);
            case compression_type::rle:
                return rle_options(options);
            case compression_type::delta:
                return delta_options(options);
        }

        // TODO: throw for unknown types (throw? assert? return empty?)
        return {};
    }

    static bool is_valid(compression_type type)
    {
        switch (type)
        {
            case compression_type::lzss:
            case compression_type::rle:
            case compression_type::delta:
                return true;
        }

        return false;
    }

    static bool is_valid(lzss_options options)
    {
        return options == lzss_options::reserved;
    }

    static bool is_valid(rle_options options)
    {
        return options == rle_options::reserved;
    }

    static bool is_valid(delta_options options)
    {
        switch (options)
        {
            case delta_options::delta8:
            case delta_options::delta16:
                return true;
        }

        return false;
    }

    static bool is_valid(compression_options options)
    {
        // TODO: consider putting validation functions into a namespace so we can confine them a bit?
        // TODO: well consider putting lots of stuff not inside the header class...
        return std::visit([](auto&& opts) { return is_valid(opts); }, options);
    }
};

}

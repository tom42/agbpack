// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <cstdint>
#include <optional>

export module agbpack:header;

namespace agbpack
{

enum class compression_type
{
    lzss = 1,
    rle = 3,
    delta = 8
};

enum class delta_options
{
    delta8 = 1,
    delta16 = 2
};

class generic_header final
{
public:
    explicit generic_header(uint32_t header_data) : m_header_data(header_data) {}

    compression_type type() const
    {
        return static_cast<compression_type>((m_header_data >> 4) & 0xf);
    }

    uint32_t options() const
    {
        return options_as<uint32_t>();
    }

    template <typename TOptions>
    TOptions options_as() const
    {
        return static_cast<TOptions>(m_header_data & 0xf);
    }

    uint32_t uncompressed_size() const
    {
        return (m_header_data >> 8) & 0xffffff;
    }

private:
    uint32_t m_header_data;
};

class delta_header final
{
public:
    uint32_t uncompressed_size() const { return m_generic_header.uncompressed_size(); }

    delta_options options() const
    {
        return m_generic_header.options_as<delta_options>();
    }

    static std::optional<delta_header> parse(uint32_t header_data)
    {
        generic_header header(header_data);

        if ((header.type() == compression_type::delta) && are_options_valid(header))
        {
            return delta_header(header);
        }

        return {};
    }
private:
    explicit delta_header(generic_header header) : m_generic_header(header) {}

    static bool are_options_valid(generic_header header)
    {
        switch (header.options_as<delta_options>())
        {
            case delta_options::delta8:
            case delta_options::delta16:
                return true;
        }

        return false;
    }

    generic_header m_generic_header;
};

class rle_header final
{
public:
    uint32_t uncompressed_size() const { return m_generic_header.uncompressed_size(); }

    static std::optional<rle_header> parse(uint32_t header_data)
    {
        generic_header header(header_data);

        if ((header.type() == compression_type::rle) && (header.options() == 0))
        {
            return rle_header(header);
        }

        return {};
    }

private:
    explicit rle_header(generic_header header) : m_generic_header(header) {}

    generic_header m_generic_header;
};

// TODO: experimental new header parsing code

class header2 final // TODO: rename
{
public:

    static std::optional<header2> parse_for_type(compression_type /*wanted_type*/, uint32_t header_data)
    {
        auto header = parse(header_data);
        // TODO: check wanted type here. If it's bad, return empty optional. Otherwise return what we got from parse().
        return header;
    }

private:
    explicit header2(compression_type type) : m_type(type) {}

    compression_type m_type;

    static std::optional<header2> parse(uint32_t header_data)
    {
        // TODO: parse compression options (depends on type), return that too.
        auto type = static_cast<compression_type>((header_data >> 4) & 0xf);
        if (!is_valid(type))
        {
            return {};
        }

        return header2(type);
    }

    static bool is_valid(compression_type type)
    {
        switch (type)
        {
            // TODO: add remaining compression types, but do so in a test first manner
            case compression_type::lzss:
            case compression_type::rle:
            //case compression_type::delta:
                return true;
        }

        return false;
    }
};

}

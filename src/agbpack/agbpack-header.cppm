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

}

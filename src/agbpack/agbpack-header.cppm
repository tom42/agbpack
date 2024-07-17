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

    uint32_t uncompressed_size() const
    {
        // TODO: does this need a test?
        // TODO: this should be factored out
        return (m_header_data >> 8) & 0xffffff;
    }

private:
    uint32_t m_header_data;
};

class delta_header final
{
public:
private:
    uint32_t m_header_data;
};

class rle_header final
{
public:
    uint32_t uncompressed_size() const { return m_generic_header.uncompressed_size(); }

    static std::optional<rle_header> parse(uint32_t header_data)
    {
        // TODO: obtaining this should be factored out
        if (static_cast<compression_type>((header_data >> 4) & 0xf) != compression_type::rle)
        {
            return {};
        }

        // TODO: obtaining this should be factored out
        if (header_data & 0xf)
        {
            return {};
        }

        return rle_header(header_data);
    }

private:
    explicit rle_header(uint32_t header_data) : m_generic_header(header_data) {}

    generic_header m_generic_header;
};

}

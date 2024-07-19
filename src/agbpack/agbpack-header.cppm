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

using compression_options = std::variant<lzss_options, rle_options, delta_options>;

class header2 final // TODO: rename
{
public:
    compression_type type() const { return m_type; }

    static std::optional<header2> parse_for_type(compression_type wanted_type, uint32_t header_data)
    {
        auto header = parse(header_data);
        if (!header || (header->type() != wanted_type))
        {
            return {};
        }

        return header;
    }

private:
    explicit header2(compression_type type, compression_options options)
        : m_type(type)
        , m_options(options)
    {}

    compression_type m_type;
    compression_options m_options;

    static std::optional<header2> parse(uint32_t header_data)
    {
        // TODO: parse compression options (depends on type), return that too.
        auto type = static_cast<compression_type>((header_data >> 4) & 0xf);
        if (!is_valid(type))
        {
            return {};
        }

        auto options = create_options(type);
        if (!options || !is_valid(*options))
        {
            return {};
        }
        // TODO: parse options too, here. Can now go and use visitor pattern

        return header2(type, *options);
    }

    static std::optional<compression_options> create_options(compression_type type)
    {
        // TODO: create option variant for each type
        // TODO: throw for unknown types (throw? assert? return empty?)
        switch (type)
        {
            case compression_type::lzss:
                return lzss_options(); // TODO: we want to pass in the value here, no?
            case compression_type::rle:
                return rle_options(); // TODO: pass in the value!
            case compression_type::delta:
                return delta_options(); // TODO: pass in the value
        }

        // TODO: do we assert here, or do we return empty? (for the time being, both would work)
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

    static bool is_valid(lzss_options /*lzss_options*/)
    {
        // TODO: actually validate lzss options
        return true;
    }

    static bool is_valid(rle_options)
    {
        // TODO: actually validate rle options
        throw "TODO: Yikes: this branch is not implemented";
    }

    static bool is_valid(delta_options)
    {
        // TODO: actually validate delta options
        throw "TODO: Yikes: this branch is not implemented";
    }

    static bool is_valid(compression_options options)
    {
        // TODO: consider putting validation functions into a namespace so we can confine them a bit?
        return std::visit([](auto&& options) { return is_valid(options); }, options);
    }
};

}

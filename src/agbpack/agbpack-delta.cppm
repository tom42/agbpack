// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <cstdint>
#include <iterator>
#include <stdexcept>
#include <vector>

export module agbpack:delta;
import :common;
import :exceptions;
import :header;

namespace agbpack
{

export class delta_decoder final
{
public:
    template <std::input_iterator InputIterator, typename OutputIterator>
    void decode(InputIterator input, InputIterator eof, OutputIterator output)
    {
        static_assert_input_type(input);

        byte_reader<InputIterator> reader(input, eof);
        auto header = header::parse_for_type(compression_type::delta, reader.read32());
        if (!header)
        {
            throw bad_encoded_data();
        }

        byte_writer<OutputIterator> writer(header->uncompressed_size(), output);
        decode8or16(header->template options_as<delta_options>(), reader, writer);
    }

private:
    template <typename InputIterator, std::output_iterator<agbpack_io_datatype> OutputIterator>
    static void decode8or16(delta_options options, byte_reader<InputIterator>& reader, byte_writer<OutputIterator>& writer)
    {
        switch (options)
        {
            case delta_options::delta8:
                generic_decode(size8, reader, writer);
                return;
            case delta_options::delta16:
                generic_decode(size16, reader, writer);
                return;
        }

        throw bad_encoded_data();
    }

    template <typename SizeTag, typename InputIterator, std::output_iterator<agbpack_io_datatype> OutputIterator>
    static void generic_decode(SizeTag, byte_reader<InputIterator>& reader, byte_writer<OutputIterator>& writer)
    {
        typename SizeTag::type current_value = 0;

        while (!writer.done())
        {
            current_value += reader.read(SizeTag());
            writer.write(SizeTag(), current_value);
        }

        reader.parse_padding_bytes();
    }
};

export class delta_encoder final
{
public:
    template <std::input_iterator InputIterator, typename OutputIterator>
    void encode(InputIterator input, InputIterator eof, OutputIterator output)
    {
        static_assert_input_type(input);

        // TODO: do we want to optimize for RandomAccessIterator?
        // TODO: write to output
        //       * For starters we can probably use the existing byte_writer, but maybe we want to have an unchecked variant?
        //       * Yeah but in the spirit of C++ you'd then make many of the write functions non-element functions, no?

        // TODO: need a way to create headers (as opposed to parsing headers)
        // TODO: what if data is too big to fit into a compression header? We should test this, no?

        // TODO: implement encoding loop: encode to temporary buffer.
        auto tmp = encode8or16(input, eof);

        // TODO: beautify: we need a tmp copy of nbytes_written, because when we write the padding bytes we'll screw up the counter
        auto tmp_siz = tmp.size();

        // TODO: add padding bytes
        auto nbytes_written = tmp.size();
        while (nbytes_written % 4 != 0)
        {
            tmp.push_back(0);
            ++nbytes_written;
        }

        // TODO: size must fit into 24 bits. who checks this?
        // TODO: to do: if the header is not valid, what do we to? Throw? And what?
        auto header = header::create(compression_type::delta, m_options, static_cast<uint32_t>(tmp_siz)); // TODO: no cast here

        // Copy header and encoded data to output
        byte_writer<OutputIterator> writer(8192, output); // TODO: unhardcode 8192? what do we want to pass here? Do we even want to pass anything?
        writer.write32(header.to_uint32_t());
        writer.write(tmp.begin(), tmp.end());
    }

    void options(delta_options options)
    {
        if (!is_valid(options))
        {
            throw std::invalid_argument("Invalid delta compression options");
        }

        m_options = options;
    }

private:
    template <typename InputIterator>
    std::vector<agbpack_u8> encode8or16(InputIterator input, InputIterator eof)
    {
        switch (m_options)
        {
            case delta_options::delta8:
                return generic_encode(size8, input, eof);
            case delta_options::delta16:
                return generic_encode(size16, input, eof);
        }

        throw "TODO: proper exception here";
    }

    template <typename SizeTag, typename InputIterator>
    std::vector<agbpack_u8> generic_encode(SizeTag, InputIterator input, InputIterator eof)
    {
        using symbol_type = typename SizeTag::type;

        std::vector<agbpack_u8> encoded_data;
        byte_reader<InputIterator> reader(input, eof);
        // TODO: it's really unfortunate if we have to pass a size here (unhardcode/remove 8192. see also todo below)
        // TODO: decltype: temporary hack to suppress clang++ warning about CTAD
        byte_writer<decltype(back_inserter(encoded_data))> writer(8192, back_inserter(encoded_data));

        symbol_type old_value = 0;
        while (!reader.eof())
        {
            symbol_type current_value = reader.read(SizeTag());
            symbol_type delta = current_value - old_value;
            old_value = current_value;
            writer.write(SizeTag(), delta);
        }

        return encoded_data;
    }

    delta_options m_options = delta_options::delta8;
};

}

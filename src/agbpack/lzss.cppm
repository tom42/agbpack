// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <utility>
#include <vector>

export module agbpack:lzss;
import :common;
import :exceptions;
import :header;

namespace agbpack
{

// Note: In VS 2022, MSVC for x86 bugs if we fully qualify std::size_t in the lzss_sliding_window class template.
// We work around this by importing it (referring to C's global size_t would probably work too).
using size_t = std::size_t;
using std::vector;

inline constexpr size_t minimum_offset = 1;
inline constexpr size_t maximum_offset = 4096;
inline constexpr size_t minimum_vram_safe_offset = 2;
inline constexpr size_t minimum_match_length = 3;
inline constexpr size_t maximum_match_length = 18;

constexpr size_t get_minimum_offset(bool vram_safe)
{
    return vram_safe ? minimum_vram_safe_offset : minimum_offset;
}

// Sliding window for LZSS decoder. Used when the output iterator does not allow random access.
// * Maintains an internal write position which wraps around when the window is written to.
// * Allows reading relative to the write position. The final read position wraps around.
//   Note: the sliding window is not initialized. Reading from a position that has not yet
//   been written to returns uninitialized data.
template <size_t Size>
class lzss_sliding_window final
{
public:
    agbpack_u8 read8(size_t offset)
    {
        assert_read_allowed(offset);
        return m_buf[(m_nbytes_written - offset) & index_mask];
    }

    void write8(agbpack_u8 byte)
    {
        m_buf[m_nbytes_written & index_mask] = byte;
        ++m_nbytes_written;
    }

private:
    static_assert(std::popcount(Size) == 1, "Size must be a power of 2 for index calculations using operator & to work");
    static constexpr size_t index_mask = Size - 1;

    void assert_read_allowed([[maybe_unused]] size_t offset)
    {
        // Required for g++ 14.2.0, which ignores [[maybe_unused]] here
        std::ignore = offset;

        assert((m_nbytes_written > 0) && "Cannot read from empty sliding window");
        assert((offset > 0) && "Cannot read from current write position");
        assert((offset <= Size) && "Offset is too big");
        assert((offset <= m_nbytes_written) && "Offset is too big for amount of data currently in sliding window");
    }

    size_t m_nbytes_written = 0;
    std::array<agbpack_u8, Size> m_buf;
};

// TODO: attempt at creating a concept for a decoder sink/receiver. REVIEW!
// TODO: spelling of concept names
// TODO: so, what is it now? A receiver? Or a sink? Or what?
template <typename Receiver>
concept lzss_decoder_sink = requires(Receiver receiver, agbpack_u8 byte, size_t size)
{
    { receiver.tags(byte) };
    { receiver.literal(byte) };
    { receiver.reference(size, size) };
};

// General case LZSS decoder output receiver.
// Works with any kind of output iterator, including those that cannot be read from.
// In order to decode references it maintains an internal sliding window.
template <typename OutputIterator>
class lzss_decoder_output_receiver final
{
public:
    explicit lzss_decoder_output_receiver(OutputIterator output)
        : m_writer(output) {}

    void tags(agbpack_u8) {}

    void literal(agbpack_u8 literal)
    {
        write8(literal);
    }

    void reference(size_t length, size_t offset)
    {
        while (length--)
        {
            auto byte = m_window.read8(offset);
            write8(byte);
        }
    }

private:
    void write8(agbpack_u8 byte)
    {
        m_writer.write8(byte);
        m_window.write8(byte);
    }

    unbounded_byte_writer<OutputIterator> m_writer;
    lzss_sliding_window<maximum_offset> m_window;
};

// Specialized LZSS decoder output receiver for random access iterators.
// Does not need memory for a separate sliding window because references can be read from the output buffer.
template <std::random_access_iterator RandomAccessIterator>
class lzss_decoder_output_receiver<RandomAccessIterator> final
{
public:
    explicit lzss_decoder_output_receiver(RandomAccessIterator output)
        : m_output(output)
    {}

    void tags(agbpack_u8) {}

    void literal(agbpack_u8 literal)
    {
        write8(literal);
    }

    void reference(size_t length, size_t offset)
    {
        while (length--)
        {
            auto byte = *(m_output - make_signed(offset));
            write8(byte);
        }
    }

private:
    void write8(agbpack_u8 byte)
    {
        *m_output++ = byte;
    }

    RandomAccessIterator m_output;
};

export class lzss_decoder final
{
public:
    template <std::input_iterator InputIterator, typename OutputIterator>
    void decode(InputIterator input, InputIterator eof, OutputIterator output)
    {
        static_assert_input_type<InputIterator>(); // TODO: probably we want to either remove this or extend it with the output iterator?

        byte_reader<InputIterator> reader(input, eof);
        lzss_decoder_output_receiver<OutputIterator> receiver(output);

        decode_internal(reader, receiver);
    }

    template <std::input_iterator InputIterator, lzss_decoder_sink LzssDecoderSink>
    void decode(InputIterator input, InputIterator eof, LzssDecoderSink sink)
    {
        static_assert_input_type<InputIterator>();
        byte_reader<InputIterator> reader(input, eof);
        decode_internal(reader, sink);
    }

    // When VRAM safety is enabled in the decoder, the decoder throws if the encoded data is not VRAM safe.
    // Use this when you want to verify that data is VRAM safe.
    void vram_safe(bool enable)
    {
        m_vram_safe = enable;
    }

    bool vram_safe() const
    {
        return m_vram_safe;
    }

private:
    // TODO: probably we can't tell overloads from eachother, so this needs a different name, no?
    //       * Well we could generalize this and give it two args, no? A sink and a source...
    //       * Well we already have the source: it's the ByteReader...but that's not yet exported, no?
    // TODO: do we even care about InputIterator here? Should we just use TByteReader? Should there be a concept byte_reader?
    template <std::input_iterator InputIterator, typename LzssReceiver>
    void decode_internal(byte_reader<InputIterator>& reader, LzssReceiver& receiver) // TODO: arg types (const? reference?)
    {
        static_assert_input_type<InputIterator>();

        auto header = header::parse_for_type(compression_type::lzss, read32(reader));
        if (!header)
        {
            throw decode_exception();
        }

        // TODO: still not happy with the type name lzss_receiver / LzssReceiver.
        //       Thing is, this receives stuff from a *decoder*. Moreover, an encoder may also have a receiver, and it's interface may look more or less the same (see lzss_bitstream_writer below)
        //       Maybe think again about that name, but leave the bitstream writer below alone, since we're not going to give the encoder the same debug output capability for the time being.

        unsigned int tag_mask = 0;
        agbpack_u8 tags = 0;
        size_t nbytes_written = 0;

        while (nbytes_written < header->uncompressed_size())
        {
            tag_mask >>= 1;
            if (!tag_mask)
            {
                tags = read8(reader);
                tag_mask = 0x80;
                receiver.tags(tags);
            }

            if (tags & tag_mask)
            {
                auto b0 = read8(reader);
                auto b1 = read8(reader);
                size_t length = ((b0 >> 4) & 0xf) + minimum_match_length;
                size_t offset = (((b0 & 0xfu) << 8) | b1) + minimum_offset;

                assert(in_closed_range(length, minimum_match_length, maximum_match_length) && "lzss_decoder is broken");
                assert(in_closed_range(offset, minimum_offset, maximum_offset) && "lzss_decoder is broken");

                // TODO: call the following three from one function, details are not important here (orly?)
                throw_if_not_vram_safe(offset);
                throw_if_outside_sliding_window(offset, nbytes_written);
                throw_if_reference_overflows_uncompressed_size(length, nbytes_written, header->uncompressed_size());

                receiver.reference(length, offset);
                nbytes_written += length;
            }
            else
            {
                receiver.literal(read8(reader));
                ++nbytes_written;
            }
        }

        parse_padding_bytes(reader);
    }

    void throw_if_not_vram_safe(size_t offset)
    {
        if (offset < get_minimum_offset(m_vram_safe))
        {
            throw decode_exception("encoded data is not VRAM safe");
        }
    }

    static void throw_if_outside_sliding_window(size_t offset, size_t nbytes_written)
    {
        if (offset > nbytes_written)
        {
            throw decode_exception("reference outside of sliding window");
        }
    }

    static void throw_if_reference_overflows_uncompressed_size(size_t length, size_t nbytes_written, size_t uncompressed_size)
    {
        if ((nbytes_written + length) > uncompressed_size)
        {
            throw decode_exception();
        }
    }

    bool m_vram_safe = false;
};

AGBPACK_EXPORT_FOR_UNIT_TESTING
class match final
{
public:
    explicit match(size_t length, size_t offset)
        : m_length(length)
        , m_offset(offset)
    {}

    size_t length() const { return m_length; }

    size_t offset() const { return m_offset; }

    bool operator==(const match&) const = default;

private:
    size_t m_length;
    size_t m_offset;
};

// Simple implementation using two nested loops each doing linear search.
AGBPACK_EXPORT_FOR_UNIT_TESTING
class match_finder final // TODO: rename this to greedy_match_finder?
{
public:
    // Note: match_finder does not own input
    explicit match_finder(const vector<agbpack_u8>& input, size_t minimum_match_offset)
        : m_input(input)
        , m_minimum_match_offset(minimum_match_offset)
    {}

    // TODO: review this again (compare against reference search)
    match find_match(size_t current_position) const
    {
        match best_match(0, 0); // TODO: start with length=0, or length=2 (minimum_match_length-1)?

        // TODO: implement this.
        //       Basic idea: two nested loops.
        //       * Outer loop: search backwards through sliding window (towards larger offset, away from current position => Nope, wrong, we start at longest offset and keep doing so while it is not 0)
        //         * Inner loop: search forwards, increasing string length.
        //           * Two problems here:
        //             * The maximum match length to search for is of course maximum_match_length, but towards the end of the buffer it may be shorter

        size_t offset = std::min(current_position, maximum_offset);

        for (; offset > m_minimum_match_offset; --offset)
        {
            // TODO: close to end of input it is pointless iterating all over [0, maximum_match_length] if that exceeds input. But is it worth optimizing this?
            size_t length = 0;
            for (; length < maximum_match_length; ++length)
            {
                if (current_position + length >= m_input.size())
                {
                    // End of lookahead reached, abort search
                    break;
                }

                if (m_input[current_position + length] != m_input[current_position + length - offset])
                {
                    // Characters differ, abort search
                    break;
                }
            }

            if (length > best_match.length())
            {
                best_match = match(length, offset);
                if (length >= maximum_match_length)
                {
                    // Found a match of maximum length, no need to search any further.
                    break;
                }
            }
        }

        return best_match;
    }

private:
    const vector<agbpack_u8>& m_input;
    size_t m_minimum_match_offset;
};

AGBPACK_EXPORT_FOR_UNIT_TESTING
class lzss_bitstream_writer final
{
public:
    // Note: lzss_bitstream_writer does not own encoded_data
    explicit lzss_bitstream_writer(vector<agbpack_u8>& encoded_data)
        : m_encoded_data(encoded_data)
    {}

    void write_literal(agbpack_u8 literal)
    {
        write_tag(false);
        m_encoded_data.push_back(literal);
    }

    void write_reference(size_t length, size_t offset)
    {
        assert(in_closed_range(length, minimum_match_length, maximum_match_length));
        assert(in_closed_range(offset, minimum_offset, maximum_offset));

        write_tag(true);

        auto b0 = ((length - minimum_match_length) << 4) | ((offset - minimum_offset) >> 8);
        auto b1 = (offset - minimum_offset) & 255;

        m_encoded_data.push_back(static_cast<agbpack_u8>(b0));
        m_encoded_data.push_back(static_cast<agbpack_u8>(b1));
    }

private:
    void write_tag(bool is_reference)
    {
        m_tag_bitmask >>= 1;
        if (!m_tag_bitmask)
        {
            // Tag byte is full.
            // Allocate space for a new one in the output and remember its position.
            m_tag_bitmask = 0x80;
            m_encoded_data.push_back(0);
            m_tag_byte_position = m_encoded_data.size() - 1;
        }

        if (is_reference)
        {
            m_encoded_data[m_tag_byte_position] |= m_tag_bitmask;
        }
    }

    agbpack_u8 m_tag_bitmask = 0;
    size_t m_tag_byte_position = 0;
    vector<agbpack_u8>& m_encoded_data;
};

export class lzss_encoder final
{
public:
    template <std::input_iterator InputIterator, typename OutputIterator>
    void encode(InputIterator input, InputIterator eof, OutputIterator output)
    {
        static_assert_input_type<InputIterator>();

        const auto uncompressed_data = vector<agbpack_u8>(input, eof);
        const auto encoded_data = encode_internal(uncompressed_data);
        const auto header = header::create(lzss_options::reserved, uncompressed_data.size());

        // Copy header and encoded data to output
        unbounded_byte_writer<OutputIterator> writer(output);
        write32(writer, header.to_uint32_t());
        write(writer, encoded_data.begin(), encoded_data.end());
        write_padding_bytes(writer);
    }

    void vram_safe(bool enable)
    {
        m_vram_safe = enable;
    }

    bool vram_safe() const
    {
        return m_vram_safe;
    }

private:
    vector<agbpack_u8> encode_internal(const vector<agbpack_u8>& input)
    {
        vector<agbpack_u8> encoded_data;
        match_finder match_finder(input, get_minimum_offset(m_vram_safe) - 1); // TODO: unhardcode. What's somewhat ugly: match_finder uses zero based offfset, whereas global constant uses one based offset
        lzss_bitstream_writer writer(encoded_data);

        size_t current_position = 0;
        while (current_position < input.size())
        {
            auto match = match_finder.find_match(current_position);

            if (match.length() >= minimum_match_length)
            {
                writer.write_reference(match.length(), match.offset());
                current_position += match.length();
            }
            else
            {
                writer.write_literal(input[current_position]);
                current_position += 1;
            }
        }

        return encoded_data;
    }

private:
    bool m_vram_safe = false;
};

// TODO: put this into a cpp module instead of having it inline?
// TODO: better to use the first longest match or the last one? (for subsequent entropy coder?)
AGBPACK_EXPORT_FOR_UNIT_TESTING
inline vector<match> find_longest_matches(const vector<agbpack_u8>& input, bool vram_safe)
{
    vector<match> longest_matches;
    longest_matches.reserve(input.size());

    // TODO: this is somewhat unfortunate: for literals and stuff this returns silly data, e.g. (l=0, m=0)
    //       This may be OK for the greedy encoder, but here we want literals to have a length=1.
    //       We can of course document that anything with match_length < minimum_match_length has an invalid offset, but somehow I dislike this, no?
    //       * Problem: bloom's document says
    //           "Let cml[n] be the chosen match length for byte n; (1 <= cml[n] <= ml[n]),
    //            *where 1 indicates a literal*"
    //         This is not how match_finder works, which may return 0 for a literal. OUCH.
    //       * The problem is that we never properly defined what the fields of match mean in what case and that
    //         for matches with length < 3 match finder returns weird results. Things do work if we interpret
    //         anything with length < minimum_match_length as 'encoder a literal', which is actually what both
    //         lzss_encoder and optimal_lzss_encoder do
    match_finder match_finder(input, get_minimum_offset(vram_safe) - 1); // TODO: this subtraction is REALLY ugly (see above where already have this)

    for (size_t i = 0; i < input.size(); ++i)
    {
        auto m = match_finder.find_match(i);
        // TODO: the if/else here is a bit of a kludge. Or is it?
        //       Actually, maybe it is not: anything with a length < 3 we encode as a literal, and offset=0 refers to itself!
        if (m.length() < minimum_match_length)
        {
            longest_matches.push_back(match(1, 0));
        }
        else
        {
            longest_matches.push_back(m);
        }
    }

    return longest_matches;
}

// TODO: put this into a cpp module instead of having it inline?
AGBPACK_EXPORT_FOR_UNIT_TESTING
inline vector<match> choose_matches(const vector<match>& ml)
{
    // TODO: copy explanation from optimal_lzss_encoder to here?
    if (ml.empty())
    {
        return {};
    }

    // TODO: does it even make sense for cml to be an array of matches? ml only contains the longest match, no? We don't know what the shorter ones would be, no?
    //       Umm...if there is a long match at offset O, then there are shorter matches at the very same offset too, no?
    vector<size_t> out(ml.size(), 0xbaadf00du); // TODO: initialized for debugging
    vector<match> cml(ml.size(), match(0xbaadf00du, 0xbaadf00du)); // TODO: initialized for debugging

    // A.
    const size_t M = 17; // TODO: cost of a match. Should we incorporate the tag bit or not? of course we should: a single match costs 16+1=17 bits, but 2 literals cost 2*(8+1)=18 bits so are more expensive
    const size_t L = 9; // TODO: cost of a match. Should we incorporate the tag bit or not?
    const size_t N = ml.size() - 1;
    out[N] = 0; // TODO: should this really be 0, or the cost for a literal?
    cml[N] = match(1, 0);
    auto c = N; // TODO: that we use N here instead of N-1, but our loop condition will fix it (if N=0, then we will not enter the loop body at all, which is the case for input length=1)

    // B.
    while (c--)
    {
        // TODO: note down that array indices in output are one based for the sake of simplicity. Index=0 is never used (should we crap this into a class?)
        // TODO: should we hoist ouput out of the loop?
        std::array<size_t, maximum_match_length + 1> output;
        for (size_t l = 2; l <= ml[c].length(); ++l)
        {
            output[l] = M + out[c + l - 1]; // TODO: ugh: this overflows: do we need a -1 here somewhere because we are not one-bsed?
        }
        // TODO: WARNING!!!!!!!!! The following and the line above may have mixed up array indexing!
        // TODO: !!! UGH: this is a complete mess: maybe redo it and get the array indexing right?
        output[1] = L + out[c + 1]; // TODO: above we've hacked in a -1. Do we need one here too? Probably, no? (Nah, this loop we calculate out[c], based on out[c + 1]


        // TODO: find the l which minimizes
        auto begin = &output[1];
        // TODO: important: iterator ranges are half open, so we need a + 1 here, since we want to look the costs of l=1 .. l=ml[c]
        // TODO: maybe do not call this end then, since it is NOT the end of the search range?
        auto end = &output[ml[c].length()];
        auto l = make_unsigned(std::distance(&output[0], std::min_element(begin, end + 1)));

        // TODO: update out
        // TODO: update cml correctly
        cml[c] = match(l, ml[c].offset());
        out[c] = output[l];
    }

    return cml;
}

export class optimal_lzss_encoder final
{
public:
    template <std::input_iterator InputIterator, typename OutputIterator>
    void encode(InputIterator input, InputIterator eof, OutputIterator output)
    {
        static_assert_input_type<InputIterator>();

        // Based on https://cbloom.com/algs/dictionary.html
        //
        // "We run through the entire file and create an array, ml[n], storing the maximum match length at each position.
        // The trick is now to run *backwards* through the file to find the optimal way to choose literals and matches throughout.
        // We assume literals and matches are coded in L and M bits.
        //
        // We run backwards, creating several arrays as we go. At each byte, we store the best choice: literal or match,
        // and the number of bytes needed to code all the data from that point on to the end of the file.
        // Then, at some preceding byte we choose literal or match based on the minimum total output length resulting.
        // Obviously, when we get back to the first byte, we have chosen the optimal coding for the file. Here's how it works:
        //
        // Let out[n] be the total output length to code from byte n to the EOF.
        // Let cml[n] be the chosen match length for byte n; (1 <= cml[n] <= ml[n]), where 1 indicates a literal
        // Let N be the last byte in the file, and c the current pos.
        //
        // A.    out[N] = 0
        //       cml[N] = 1
        //       c = N - 1
        //
        // B.	 for all 2 <= l <= ml[c], compute:
        //           output[l] = M + out[c + l]
        //       output[1] = L + out[c + 1]
        //       find the 1 <= l <= ml[c] which minimizes output[l]
        //
        //       cml[c] = l
        //       out[c] = output[l]
        //       c -= 1
        //
        // C.    if c > 0 , goto B
        //
        //	     create the output:
        //           c = 0
        //           send cml[c]
        //           c += cml[c]
        //           repeat

        const auto uncompressed_data = vector<agbpack_u8>(input, eof);
        const auto ml = find_longest_matches(uncompressed_data, m_vram_safe);
        const auto cml = choose_matches(ml);
        const auto encoded_data = write_bitstream(cml, uncompressed_data);
        const auto header = header::create(lzss_options::reserved, uncompressed_data.size());

        // TODO: the following mantra should go into some sort of helper, no? We have it by now multiple times...
        // Copy header and encoded data to output
        unbounded_byte_writer<OutputIterator> writer(output);
        write32(writer, header.to_uint32_t());
        write(writer, encoded_data.begin(), encoded_data.end());
        write_padding_bytes(writer);
    }

    void vram_safe(bool enable)
    {
        m_vram_safe = enable;
    }

    bool vram_safe() const
    {
        return m_vram_safe;
    }

private:
    static vector<agbpack_u8> write_bitstream(const vector<match>& chosen_matches, const vector<agbpack_u8>& uncompressed_data)
    {
        assert(chosen_matches.size() == uncompressed_data.size());

        vector<agbpack_u8> bitstream;
        lzss_bitstream_writer writer(bitstream);

        auto u = uncompressed_data.begin();
        for (auto c = chosen_matches.begin(); c < chosen_matches.end(); )
        {
            if (c->length() < minimum_match_length)
            {
                writer.write_literal(*u);
                ++u;
                ++c;
            }
            else
            {
                writer.write_reference(c->length(), c->offset());
                u += make_signed(c->length()); // We need c to update u, so update u first, then c!
                c += make_signed(c->length());
            }
        }

        return bitstream;
    }

    bool m_vram_safe = false;
};

}

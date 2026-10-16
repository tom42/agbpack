// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <cstdlib>
#include <vector>
#include "clownlzss/clownlzss.h"

module agbpack;

namespace agbpack
{

using std::vector;

namespace
{

struct match_deleter final
{
    void operator()(ClownLZSS_Match* const ptr)
    {
        std::free(ptr);
    }
};

using clownlzss_matches = std::unique_ptr<ClownLZSS_Match[], match_deleter>;

size_t get_match_cost(const size_t, const size_t length, void* const)
{
    if (length < minimum_match_length)
    {
        return 0;
    }

    return match_cost;
}

size_t get_match_cost_vram_safe(const size_t distance, const size_t length, void* const)
{
    if ((length < minimum_match_length) || (distance < minimum_vram_safe_offset))
    {
        return 0;
    }

    return match_cost;
}

std::pair<clownlzss_matches, size_t> find_optimal_matches(const vector<agbpack_u8>& uncompressed_data, bool vram_safe)
{
    size_t total_matches;
    auto match_cost_callback = vram_safe ? get_match_cost_vram_safe : get_match_cost;

    ClownLZSS_Match* matches_pointer = nullptr;
    if (!ClownLZSS_FindOptimalMatches(
        filler_value,
        maximum_match_length,
        maximum_match_distance,
        nullptr,
        literal_cost,
        match_cost_callback,
        uncompressed_data.data(),
        bytes_per_value,
        uncompressed_data.size() / bytes_per_value,
        &matches_pointer,
        &total_matches,
        nullptr))
    {
        throw encode_exception("optimal LZSS encoding failed. That should not happen, unless the system is extremely low on memory");
    }
    clownlzss_matches matches(matches_pointer); // Now wrap raw pointer before doing anything else!

    return std::make_pair(std::move(matches), total_matches);
}

vector<agbpack_u8> encode_matches(const vector<agbpack_u8>& uncompressed_data, const clownlzss_matches& matches, size_t total_matches)
{
    vector<agbpack_u8> encoded_data;
    lzss_bitstream_writer writer(encoded_data);

    for (const auto& match : std::ranges::subrange(&matches[0], &matches[total_matches]))
    {
        if (CLOWNLZSS_MATCH_IS_LITERAL(&match))
        {
            writer.write_literal(uncompressed_data[match.destination]);
        }
        else
        {
            writer.write_reference(match.length, match.destination - match.source);
        }
    }

    return encoded_data;
}

}

// TODO: explain somewhere why this is in a cpp file?
//       * Yes but first
//         * Do all the installation related stuff listed in TODO.md
//           * Library target alias thing?
//           * In particular, archive the test project we use for installation testing, so that we don't have to create it over and over again.
vector<agbpack_u8> optimal_lzss_encoder::encode_internal(const vector<agbpack_u8>& uncompressed_data)
{
    if (uncompressed_data.empty())
    {
        // Code further on does not work well with zero-sized input.
        return {};
    }

    const auto [matches, total_matches] = find_optimal_matches(uncompressed_data, m_vram_safe);
    return encode_matches(uncompressed_data, matches, total_matches);
}

}

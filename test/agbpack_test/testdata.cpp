// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <filesystem>
#include <fstream>
#include <iterator>
#include <system_error>
#include "agbpack_test_config.hpp"
#include "testdata.hpp"

namespace
{

std::ifstream open_binary_file(const std::filesystem::path& name)
{
    // The exceptions thrown by ifstream when opening fails have rather useless error messages.
    // For instance, MSVC throws an exception with the following message: 'ios_base::failbit set: iostream stream error'.
    // So we don't use stream exceptions and try our luck with errno and std::system_error.
    std::ifstream f(name, std::ios_base::binary);

    if (!f)
    {
        auto error = errno;
        throw std::system_error(error, std::generic_category(), "Could not open " + name.string());
    }

    // This is required to correctly read binary files using some APIs, e.g. std::istream_iterator.
    f.unsetf(std::ios::skipws);

    return f;
}

std::uintmax_t get_file_size(const std::filesystem::path& name)
{
    std::error_code ec;
    auto size = std::filesystem::file_size(name, ec);
    if (ec)
    {
        throw std::runtime_error("Could not determine size of " + name.string() + ": " + ec.message());
    }

    return size;
}

}

namespace agbpack_test
{

// TODO: review
// TODO: note: in shrinkler test we test whether the stream is good or eof. Maybe better than what we have here?
std::vector<unsigned char> read_testdata_file(const std::string& basename)
{
    const auto name = std::filesystem::path(agbpack_test_testdata_directory) / basename;

    auto filestream = open_binary_file(name);
    auto filesize = get_file_size(name);

    // Create vector with sufficient capacity to hold entire file.
    std::vector<unsigned char> data;
    // TODO: check filesize fits into size_t before casting
    data.reserve(static_cast<std::size_t>(filesize)); // TODO: that does not work on 32 bit systems, does it?

    // Read entire file
    data.insert(
        data.begin(),
        std::istream_iterator<unsigned char>(filestream),
        std::istream_iterator<unsigned char>());

    // Sanity check
    if (data.size() != filesize)
    {
        throw std::runtime_error("Could not read entire content of file " + name.string());
    }

    return data;
}

}

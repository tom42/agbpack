// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
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
std::vector<unsigned char> read_testdata_file(const std::string& basename)
{
    const auto name = std::filesystem::path(agbpack_test_testdata_directory) / basename;

    auto filestream = open_binary_file(name);
    auto filesize = get_file_size(name);

    // Create vector with sufficient capacity to hold entire file.
    std::vector<unsigned char> data;
    if (filesize > std::numeric_limits<std::size_t>::max())
    {
        throw std::runtime_error("Cannot read file " + name.string() + ": file is too big");
    }
    data.reserve(static_cast<std::size_t>(filesize));

    // Read entire file
    data.insert(
        data.begin(),
        std::istream_iterator<unsigned char>(filestream),
        std::istream_iterator<unsigned char>());

    // Sanity check
    if (filestream.bad() || (data.size() != filesize)) // TODO: port that over into the shrinkler-gba test suite
    {
        throw std::runtime_error("Could not read entire content of file " + name.string());
    }

    return data;
}

}

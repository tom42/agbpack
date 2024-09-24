// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <filesystem>
#include <iterator>
#include <system_error>
#include "agbpack_test_config.hpp"
#include "testdata.hpp"

namespace agbpack_test
{

std::size_t get_file_size(const std::string& path)
{
    std::error_code ec;
    auto size = std::filesystem::file_size(path, ec);
    if (ec)
    {
        throw std::runtime_error("could not determine size of " + path + ": " + ec.message());
    }

    // File size may not fit into std::size_t (e.g. on 32 bit systems), so we have to cast.
    // The cast should be harmless considering the size of files we test with and
    // the target CPUs we're aiming for.
    return static_cast<std::size_t>(size);
}

std::ifstream open_binary_file(const std::string& path)
{
    // The exceptions thrown by ifstream when opening fails have rather useless error messages.
    // For instance, MSVC throws an exception with the following message: 'ios_base::failbit set: iostream stream error'.
    // So we don't use stream exceptions and try our luck with errno and std::system_error.
    std::ifstream file(path, std::ios_base::binary);

    if (!file)
    {
        auto error = errno;
        throw std::system_error(error, std::generic_category(), "could not open " + path);
    }

    // This is required to correctly read binary files using some APIs, e.g. std::istream_iterator.
    file.unsetf(std::ios::skipws);

    return file;
}

std::string get_testfile_path(const std::string& basename)
{
    return (std::filesystem::path(agbpack_test_testdata_directory) / basename).string();
}

const std::vector<unsigned char> read_file(const std::string& basename)
{
    const auto name = std::filesystem::path(agbpack_test_testdata_directory) / basename;

    auto filestream = open_binary_file(name.string());
    auto filesize = get_file_size(name.string());

    // Create vector with sufficient capacity to hold entire file.
    std::vector<unsigned char> data;
    data.reserve(filesize);

    // Read entire file
    data.insert(
        data.begin(),
        std::istream_iterator<unsigned char>(filestream),
        std::istream_iterator<unsigned char>());

    // Sanity check
    if (filestream.bad() || (data.size() != filesize))
    {
        throw std::runtime_error("could not read entire content of file " + name.string());
    }

    return data;
}

}

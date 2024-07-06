// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <filesystem>
#include <fstream>
#include <system_error>
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

std::vector<unsigned char> read_testdata_file(const std::string& /*basename*/)
{
    return std::vector<unsigned char>();
}

}

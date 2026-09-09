// SPDX-FileCopyrightText: 2026 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <cerrno>
#include <cstdio>
#include <system_error>

module agbpacker_core;

namespace agbpacker_core
{

void fcloser::operator()(FILE* fp) const
{
    fclose(fp);
}

unique_file_ptr fcloser::open(const char* filename, const char* mode)
{
    unique_file_ptr fp(std::fopen(filename, mode));

    if (!fp)
    {
        throw std::system_error(errno, std::generic_category());
    }

    return fp;
}

unique_file_ptr fcloser::open(const std::string& filename, const char* mode)
{
    return open(filename.c_str(), mode);
}

}

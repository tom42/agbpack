// SPDX-FileCopyrightText: 2026 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <cerrno>
#include <cstdio>
#include <system_error>

module agbpacker_core;

namespace agbpacker_core
{

namespace
{

template <typename TPredicate>
void throw_system_error_if(TPredicate predicate)
{
    if (predicate())
    {
        throw std::system_error(errno, std::generic_category());
    }
}

int to_c_origin(seek_origin origin)
{
    switch (origin)
    {
        case seek_origin::set:
            return SEEK_SET;
        case seek_origin::cur:
            return SEEK_CUR;
        case seek_origin::end:
            return SEEK_END;
        default:
            throw "TODO: YIKES: (proper exception)";
    }
}

}

void fcloser::operator()(FILE* fp) const
{
    fclose(fp);
}

unique_file_ptr fcloser::open(const char* filename, const char* mode)
{
    unique_file_ptr fp(std::fopen(filename, mode));
    throw_system_error_if([&] { return !fp; });
    return fp;
}

unique_file_ptr fcloser::open(const std::string& filename, const char* mode)
{
    return open(filename.c_str(), mode);
}

file file::open(const char* filename, const char* mode)
{
    return file(filename, mode);
}

file file::open(const std::string& filename, const char* mode)
{
    return open(filename.c_str(), mode);
}

void file::seek(long offset, seek_origin origin)
{
    int result = fseek(m_file_ptr.get(), offset, to_c_origin(origin));
    throw_system_error_if([&] { return result != 0; });
}

long file::tell()
{
    long pos = ftell(m_file_ptr.get());
    throw_system_error_if([&] { return pos == -1L; });
    return pos;
}

file::file(const char* filename, const char* mode)
    : m_file_ptr(fcloser::open(filename, mode)) {}

}

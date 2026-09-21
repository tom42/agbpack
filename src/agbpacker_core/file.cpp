// SPDX-FileCopyrightText: 2026 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <cerrno>
#include <cstdio>
#include <stdexcept>
#include <system_error>
#include <vector>

module agbpacker_core;

namespace agbpacker_core
{

namespace
{

[[noreturn]] void throw_system_error()
{
    throw std::system_error(errno, std::generic_category());
}

template <typename TPredicate>
void throw_system_error_if(TPredicate predicate)
{
    if (predicate())
    {
        throw_system_error();
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
            throw std::invalid_argument("invalid seek origin");
    }
}

}

void fcloser::operator()(FILE* fp) const
{
    std::fclose(fp);
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

file file::open(zstring_view filename, const char* mode)
{
    return file(filename.c_str(), mode);
}

std::vector<unsigned char> file::read_all_bytes(zstring_view filename)
{
    auto file = open(filename, "rb");
    std::size_t filesize = file.size();

    std::vector<unsigned char> buffer(filesize);
    file.read(buffer.data(), filesize);

    return buffer;
}

void file::write_all_bytes(const std::string& /*filename*/, const std::vector<unsigned char>& /*data*/)
{
    throw "TODO: YIKES: implement";
}

void file::seek(long offset, seek_origin origin)
{
    int result = std::fseek(m_file_ptr.get(), offset, to_c_origin(origin));
    throw_system_error_if([&] { return result != 0; });
}

long file::tell()
{
    long pos = std::ftell(m_file_ptr.get());
    throw_system_error_if([&] { return pos == -1L; });
    return pos;
}

void file::read(void* buffer, std::size_t nbytes)
{
    size_t nbytes_read = std::fread(buffer, 1, nbytes, m_file_ptr.get());
    if (nbytes == nbytes_read)
    {
        return;
    }

    if (std::feof(m_file_ptr.get()))
    {
        throw std::logic_error("read past end of file");
    }

    if (std::ferror(m_file_ptr.get()))
    {
        throw_system_error();
    }

    throw std::logic_error("unknown error");
}

std::size_t file::size()
{
    long old_pos = tell();

    seek(0, seek_origin::end);
    long size = tell();

    seek(old_pos, seek_origin::set);
    return static_cast<std::size_t>(size);
}

file::file(const char* filename, const char* mode)
    : m_file_ptr(fcloser::open(filename, mode)) {}

}

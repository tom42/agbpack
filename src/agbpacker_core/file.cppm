// SPDX-FileCopyrightText: 2026 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <cstddef>
#include <cstdio>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

export module agbpacker_core:file;

namespace agbpacker_core
{

export struct fcloser;

using unique_file_ptr = std::unique_ptr<FILE, fcloser>;

export struct fcloser final
{
    void operator()(FILE* fp) const;

    static unique_file_ptr open(const char* filename, const char* mode);

    static unique_file_ptr open(const std::string& filename, const char* mode);
};

export enum class seek_origin
{
    set,
    cur,
    end
};

// TODO: do we need to export this?
AGBPACK_EXPORT_FOR_UNIT_TESTING
class zstring_view final
{
public:
    zstring_view(const char* s) : m_view(s) {}

    zstring_view(const std::string& s) : m_view(s.c_str()) {}

    const char* c_str() const
    {
        // Returning data() here is fine because we control how m_view is constructed.
        return m_view.data();
    }

private:
    std::string_view m_view;
};

// TODO: possibly create a class zstring_view and pass that, remove overloads taking const char* and string
export class file final
{
public:
    static file open(const char* filename, const char* mode);

    static file open(const std::string& filename, const char* mode);

    static std::vector<unsigned char> read_all_bytes(const char* filename);

    static std::vector<unsigned char> read_all_bytes(const std::string& filename);

    static void write_all_bytes(const std::string& filename, const std::vector<unsigned char>& data); // TODO: see whether 2nd argument should be span instead. Probably yes.

    void seek(long offset, seek_origin origin);

    long tell();

    std::size_t size();

    void read(void* buffer, std::size_t nbytes);

private:
    file(const char* filename, const char* mode);

    unique_file_ptr m_file_ptr;
};

}

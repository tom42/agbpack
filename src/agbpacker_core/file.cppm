// SPDX-FileCopyrightText: 2026 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <cstdio>
#include <memory>
#include <string>

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

export class file final
{
public:
    static file open(const char* filename, const char* mode);

    static file open(const std::string& filename, const char* mode);

    // TODO: use a scoped enum to represent origin?
    void seek(long offset, int origin);

    long tell();

private:
    file(const char* filename, const char* mode);

    unique_file_ptr m_file_ptr;
};

}

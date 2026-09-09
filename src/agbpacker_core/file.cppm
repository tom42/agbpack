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
private:
};

}

// SPDX-FileCopyrightText: 2026 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <cstdio>
#include <memory>

export module agbpacker_core:file;

namespace agbpacker_core
{

struct fcloser;

using unique_file_ptr = std::unique_ptr<FILE, fcloser>;

export struct fcloser
{
    void operator()(FILE* fp) const;

    static unique_file_ptr open(const char* filename, const char* mode);
};

}

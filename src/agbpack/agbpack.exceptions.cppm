// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <stdexcept>

export module agbpack.exceptions;

namespace agbpack
{

export class agbpack_exception : public std::runtime_error
{
protected:
    explicit agbpack_exception(const char* message) : std::runtime_error(message) {}

    virtual ~agbpack_exception() override = default;
};

export class bad_compressed_data : public agbpack_exception
{
public:
    bad_compressed_data() : agbpack_exception("compressed data is corrupt") {}

    virtual ~bad_compressed_data() override = default;
};

}

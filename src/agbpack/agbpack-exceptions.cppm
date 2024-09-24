// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <stdexcept>

export module agbpack:exceptions;

namespace agbpack
{

export class agbpack_exception : public std::runtime_error
{
protected:
    explicit agbpack_exception(const char* message) : std::runtime_error(message) {}

    virtual ~agbpack_exception() override = default;
};

export class encode_exception : public agbpack_exception
{
public:
    explicit encode_exception() : agbpack_exception("data to encode is too big") {}

    virtual ~encode_exception() override = default;
};

export class decode_exception : public agbpack_exception
{
public:
    explicit decode_exception() : agbpack_exception("encoded data is corrupt") {}

    virtual ~decode_exception() override = default;
};

}

// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <stdexcept>
#include <string>

export module agbpack:exceptions;

namespace agbpack
{

// Internal error exception.
// This is deliberately not exported and not derived from agbpack_exception.
class internal_error final : public std::logic_error
{
public:
    explicit internal_error(const char* message)
        : std::logic_error(std::string("agbpack: internal error: ") + message)
    {}
};

export class agbpack_exception : public std::runtime_error
{
protected:
    explicit agbpack_exception(const char* message) : std::runtime_error(message) {}

    explicit agbpack_exception(const std::string& message) : std::runtime_error(message) {}

    virtual ~agbpack_exception() override = default;
};

export class encode_exception : public agbpack_exception
{
public:
    explicit encode_exception(const char* message) : agbpack_exception(message) {}

    virtual ~encode_exception() override = default;
};

export class decode_exception : public agbpack_exception
{
public:
    explicit decode_exception() : agbpack_exception("encoded data is corrupt") {}

    explicit decode_exception(const char* reason)
        : agbpack_exception(std::string("encoded data is corrupt: ") + reason)
    {}

    virtual ~decode_exception() override = default;
};

}

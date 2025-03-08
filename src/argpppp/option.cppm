// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <argp.h>
#include <optional>
#include <string>

export module argpppp:option;
import :of;
import :optional_string;

namespace argpppp
{

export class option final
{
public:
    option(const optional_string& name = std::nullopt, int key = 0, const optional_string& arg = std::nullopt, of flags = of::none, const optional_string& doc = std::nullopt, int group = 0)
        // Note: we intentionally use std::nullopt rather than {} for default arguments.
        // The latter bugs with g++ 14.2 (odd linker errors which clang++ and MSVC do not produce).
        : m_name(name)
        , m_key(key)
        , m_arg(arg)
        , m_flags(flags)
        , m_doc(doc)
        , m_group(group)
    {}

    const optional_string& name() const { return m_name; }

    int key() const { return m_key; }

    const optional_string& arg() const { return m_arg; }

    of flags() const { return m_flags; }

    const optional_string& doc() const { return m_doc; }

    int group() const { return m_group; }

private:
    optional_string m_name;
    int m_key;
    optional_string m_arg;
    of m_flags;
    optional_string m_doc;
    int m_group;
};

// Converts an option to an argp_option which can be passed to argp_parse.
// Note that the option must not go out of scope while the argp_option is in use.
ARGPPPP_EXPORT_FOR_UNIT_TESTING
inline argp_option to_argp_option(const option& o)
{
    return {c_str(o.name()), o.key(), c_str(o.arg()), to_int(o.flags()), c_str(o.doc()), o.group()};
}

}

// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <algorithm>
#include <argp.h>
#include <iterator>
#include <optional>
#include <string>
#include <vector>

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

// Converts a vector of options to a vector of argp_options for use with argp_parse.
// The resulting vector is terminated by an argp_option with all fields set to zero.
// The original vector of options must not go out of scope while the vector of argp_options is in use.
ARGPPPP_EXPORT_FOR_UNIT_TESTING
inline std::vector<argp_option> to_argp_options(const std::vector<option>& options)
{
    std::vector<argp_option> argp_options;
    argp_options.reserve(options.size() + 1);
    std::transform(options.begin(), options.end(), back_inserter(argp_options), to_argp_option);
    argp_options.push_back({});
    return argp_options;
}

}

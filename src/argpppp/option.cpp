// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <algorithm>
#include <argp.h>
#include <climits>
#include <iterator>
#include <stdexcept>
#include <vector>

module argpppp;

namespace argpppp
{

namespace
{

template <std::integral T>
bool in_closed_range(T x, T min, T max)
{
    return (min <= x) && (x <= max);
}

// TODO: in principle this could do with some more testing
bool need_long_name(int key)
{
    if (key == 0)
    {
        // Special options with key=0 such as documentation options or group headers do not need a long name.
        return false;
    }

    if (!in_closed_range(key, 0, UCHAR_MAX))
    {
        // Return value of isprint is undefined in this range, so we need to filter it out first.
        // Keys in this range are not printable, so a long name is needed.
        return true;
    }

    // Options whose key is not a printable character need a long name.
    return !isprint(key);
}

}

option::option(const optional_string& name, int key, const optional_string& arg, of flags, const optional_string& doc, int group)
    : m_name(name)
    , m_key(key)
    , m_arg(arg)
    , m_flags(flags)
    , m_doc(doc)
    , m_group(group)
{
    if (need_long_name(key) && !m_name)
    {
        throw std::invalid_argument("option without printable short name needs a long name");
    }
}

argp_option to_argp_option(const option& o)
{
    return {c_str(o.name()), o.key(), c_str(o.arg()), to_int(o.flags()), c_str(o.doc()), o.group()};
}

std::vector<argp_option> to_argp_options(const std::vector<option>& options)
{
    std::vector<argp_option> argp_options;
    argp_options.reserve(options.size() + 1);
    std::transform(options.begin(), options.end(), back_inserter(argp_options), to_argp_option);
    argp_options.push_back({});
    return argp_options;
}

}

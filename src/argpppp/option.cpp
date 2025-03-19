// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <algorithm>
#include <argp.h>
#include <climits>
#include <format>
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

bool is_printable_key(int key)
{
    // key may be outside the range [0, 255].
    // Except for EOF, isprint is not defined for values outside this range.
    return in_closed_range(key, 0, UCHAR_MAX) && isprint(key);
}

bool need_long_name(int key)
{
    if (key == 0)
    {
        // Special options with key=0 such as documentation options or group headers do not need a long name.
        return false;
    }

    return !is_printable_key(key);
}

std::string get_names(const option& o)
{
    if (is_printable_key(o.key()) && o.name())
    {
        return std::format("'-{:c}' / '--{}'", o.key(), *o.name());
    }

    if (is_printable_key(o.key()))
    {
        return std::format("'-{:c}'", o.key());
    }

    if (o.name())
    {
        return std::format("'--{}'", *o.name());
    }

    throw std::invalid_argument("get_names: option has no name");
}

std::string get_default_error_message(const option& /*o*/, const char* /*arg*/)
{
    // TODO: implement, TEST
    //       * For options with arguments: ignore the fact that there are options with optional arguments. Just keep in mind: arg may be 0
    //       * For options without arguments: have a special error message
    return "meh: %s";
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

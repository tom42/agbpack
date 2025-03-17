// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <algorithm>
#include <iterator>

module argpppp;

namespace argpppp
{

option::option(const optional_string& name, int key, const optional_string& arg, of flags, const optional_string& doc, int group)
    : m_name(name)
    , m_key(key)
    , m_arg(arg)
    , m_flags(flags)
    , m_doc(doc)
    , m_group(group)
{}

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

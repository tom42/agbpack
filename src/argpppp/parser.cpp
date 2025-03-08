// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <algorithm>
#include <argp.h>
#include <optional>
#include <string>
#include <vector>

module argpppp;
// TODO: do I need to import other module partitions here? Like :optional_string or :option?

namespace argpppp
{

namespace
{

// TODO: here too: document that options must not go out of scope?
// TODO: do we want to export and unit test this?
// TODO: we could stick this into the option partition thing, no?
std::vector<argp_option> to_argp_options(const std::vector<option>& options)
{
    std::vector<argp_option> argp_options;
    argp_options.reserve(options.size() + 1);
    std::transform(options.begin(), options.end(), back_inserter(argp_options), to_argp_option);
    argp_options.push_back({});
    return argp_options;
}

}

void parser::add_option(const option& o)
{
    m_options.push_back(o);
}

void parser::doc(const optional_string& s)
{
    m_doc = s;
}

void parser::parse(int argc, char** argv)
{
    // TODO: fill in all remaining fields
    //       * parser (callback function)
    //       * args_doc
    //       * children
    //       * help_filter
    //       * argp_domain

    const auto argp_options = to_argp_options(m_options);
    const argp argp { argp_options.data(), nullptr, "TODO: args doc", c_str(m_doc), nullptr, nullptr, nullptr };

    argp_parse(&argp, argc, argv, 0, nullptr, this);
}

}

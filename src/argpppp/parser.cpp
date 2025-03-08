// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <argp.h>
#include <optional>
#include <string>
#include <vector>

module argpppp;
// TODO: do I need to import other module partitions here? Like :optional_string or :option?

namespace argpppp
{

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
    // TODO: fill in all required fields
    //       * options
    //       * parser (callback function)
    //       * args_doc
    //       * doc
    //       * children
    //       * help_filter
    //       * argp_domain

    // TODO: turn option instances into argp_option structures. Can we unit test this? It IS testworthy, isn't it?
    // TODO: do we build op this vector here, or as options are added
    std::vector<argp_option> argp_options;
    argp_options.reserve(m_options.size() + 1);
    for (const auto& o : m_options)
    {
        argp_options.push_back(to_argp_option(o));
    }
    argp_options.push_back({});

    struct argp argp { argp_options.data(), nullptr, nullptr, c_str(m_doc), nullptr, nullptr, nullptr };
    argp_parse(&argp, argc, argv, 0, nullptr, this);
}

}

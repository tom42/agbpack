// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <algorithm>
#include <argp.h>
#include <iterator>
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

void parser::args_doc(const optional_string& s)
{
    m_args_doc = s;
}

void parser::doc(const optional_string& s)
{
    m_doc = s;
}

void parser::parse(int argc, char** argv)
{
    // TODO: fill in all remaining fields
    //       * parser (callback function)

    constexpr const argp_child* children = nullptr;
    constexpr const auto help_filter = nullptr;
    constexpr const char* argp_domain = nullptr;

    const auto argp_options = to_argp_options(m_options);
    const argp argp { argp_options.data(), parse_option_static, c_str(m_args_doc), c_str(m_doc), children, help_filter, argp_domain };

    argp_parse(&argp, argc, argv, 0, nullptr, this);
}

error_t parser::parse_option(int /*key*/, char* /*arg*/, argp_state* /*state*/)
{
    // TODO: actually do something useful here:
    //       * find a registered handler for our key. If there is one, invoke it and handle its result
    //       * if there is none, handle key ourselves.
    return ARGP_ERR_UNKNOWN;
}

error_t parser::parse_option_static(int key, char* arg, argp_state* state)
{
    // TODO: at the very latest, catch all exceptions here. We could then pass them through state->input to parser::parse, which could rethrow them
    parser* p = static_cast<parser*>(state->input);
    return p->parse_option(key, arg, state);
}

}

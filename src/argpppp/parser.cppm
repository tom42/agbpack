// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <argp.h>
#include <functional>
#include <map>
#include <string>
#include <variant>
#include <vector>

export module argpppp:parser;
import :of;
import :option;
import :optional_string;
import :pf;

namespace argpppp
{

// TODO: name?
export class arg_error final {};

// TODO: add a way to supply an error. Probably want to have a class for this, not just a naked std::string
export using option_callback_result = std::variant<bool, arg_error>;
export using option_callback = std::function<option_callback_result(char*)>;

// TODO: features
//       * Does not terminate your application, unless you want it to
// TODO: document (README.md): argpppp does resource management for you, e.g. doc()
export class parser final
{
public:
    void add_option(const option& o, const option_callback& c);

    void args_doc(const optional_string& s);

    void doc(const optional_string& s);

    // TODO: should we not start working on not exiting on error?
    //       => See what exactly we did in shrinkler-gba
    int parse(int argc, char** argv, pf flags = pf::none) const;

private:
    error_t parse_option(int key, char *arg, argp_state *state) const;
    static error_t parse_option_static(int key, char *arg, argp_state *state);
    error_t handle_option_callback_result(const option_callback_result& result, int key, char* arg, const argp_state* state) const;
    error_t handle_option_callback_result_for_type(bool result, int key, char* arg, const argp_state* state) const;
    error_t handle_option_callback_result_for_type(const arg_error& error, int key, char* arg, const argp_state* state) const;

    optional_string m_args_doc;
    optional_string m_doc;
    std::vector<option> m_options;
    std::map<int, option_callback> m_callbacks;
};

export inline void add_option(parser& p, const option& o, const option_callback& c)
{
    p.add_option(o, c);
}

export inline void add_header(parser& p, const std::string& s, int group = 0)
{
    p.add_option(option({}, 0, {}, of::none, s, group), {});
}

}

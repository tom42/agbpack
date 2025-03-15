// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <argp.h>
#include <functional>
#include <map>
#include <string>
#include <vector>

export module argpppp:parser;
import :of;
import :option;
import :optional_string;

namespace argpppp
{

export using option_callback = std::function<bool(char*)>;

// TODO: this could use the program_name thing above too, no? (well do we DO want to have the program name thing at all?)
// TODO: features
//       * Does not terminate your application, unless you want it to
//       * Properly prints your program name, in any damn case
// TODO: document (README.md): argpppp does resource management for you, e.g. doc()
export class parser final
{
public:
    // TODO: rename this to add, so that using it becomes parser.add(option, ...)?
    void add_option(const option& o, const option_callback& c);

    void args_doc(const optional_string& s);

    void doc(const optional_string& s);

    // TODO: should this not get a return value?
    // TODO: should we not start working on not exiting on error?
    //       => See what exactly we did in shrinkler-gba
    void parse(int argc, char** argv);

private:
    error_t parse_option(int key, char *arg, argp_state *state);
    static error_t parse_option_static(int key, char *arg, argp_state *state);

    optional_string m_args_doc;
    optional_string m_doc;
    std::vector<option> m_options;
    std::map<int, option_callback> m_callbacks;
};

// TODO: put helper methods into cpp file? And possibly also their own cppm file?

export inline void add_header(parser& p, const std::string& s, int group = 0)
{
    p.add_option(option({}, 0, {}, of::none, s, group), {});
}

}

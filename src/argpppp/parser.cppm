// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <argp.h>
#include <string>
#include <vector>

export module argpppp:parser;
import :of;
import :option;
import :optional_string;

namespace argpppp
{

// TODO: how to deal with exceptions? Swallowing them kind of sucks too, no? (Yes but then, since they're going through C code, leaks will happen anyway...)
//        => Well we could store them and rethrow them once argp_parse returns, no? Would that not be the cleanest thing?
// TODO: this could use the program_name thing above too, no?
// TODO: features
//       * Does not terminate your application, unless you want it to
//       * Properly prints your program name, in any damn case
// TODO: document (README.md): argpppp does resource management for you, e.g. doc()
export class parser final
{
public:
    // TODO: obviously this lacks the callback
    // TODO: test this
    void add_option(const option& o);

    void args_doc(const optional_string& s);

    void doc(const optional_string& s);

    void parse(int argc, char** argv);

private:
    static error_t parse_option_static(int key, char *arg, argp_state *state);

    optional_string m_args_doc;
    optional_string m_doc;
    std::vector<option> m_options;
};

// TODO: put into cpp file?
export inline void add_header(parser& p, const std::string& s, int group = 0)
{
    p.add_option(option({}, 0, {}, of::none, s, group));
}

}

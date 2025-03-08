// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <argp.h>
#include <string>
#include <vector>

// TODO: API documentation
//       * program_name (once it's settled down)

export module argpppp;
export import :of;
export import :option;
export import :optional_string;

namespace argpppp
{

export std::string program_name(const char* argv0);

// TODO: how to deal with exceptions? Swallowing them kind of sucks too, no? (Yes but then, since they're going through C code, leaks will happen anyway...)
// TODO: this could use the program_name thing above too, no?
// TODO: features
//       * Does not terminate your application, unless you want it to
//       * Properly prints your program name, in any damn case
// TODO: document (README.md): argpppp does resource management for you, e.g. doc()
export class parser final
{
public:
    // TODO: do we test this? And how?
    void doc(const optional_string& s)
    {
        m_doc = s;
    }

    // TODO: obviously this lacks the callback
    // TODO: test this
    void add_option(const option& o)
    {
        m_options.push_back(o);
    }

    void parse(int argc, char** argv)
    {
        // TODO: fill in all required fields
        //       * options
        //       * parser (callback function)
        //       * args_doc
        //       * doc
        //       * children
        //       * help_filter
        //       * argp_domain
        // TODO: move parser implementation to cpp file

        // TODO: turn option instances into argp_option structures. Can we unit test this? It IS testworthy, isn't it?
        // TODO: do we build op this vector here, or as options are added
        std::vector<argp_option> argp_options;
        argp_options.reserve(m_options.size());
        for (const auto& o : m_options)
        {
            argp_options.push_back(to_argp_option(o));
        }
        argp_options.push_back({});

        struct argp argp { argp_options.data(), nullptr, nullptr, c_str(m_doc), nullptr, nullptr, nullptr };
        argp_parse(&argp, argc, argv, 0, nullptr, this);
    }

private:
    optional_string m_doc;
    std::vector<option> m_options;
};

// TODO: put into cpp file?
// TODO: aaactually, taking optional_string here makes no sense:
//       If somebody goes and calls this with an empty optional, then we get an argp_option that most likely does nothing useful,
//       or, if group is 0, terminates the list of options.
//       => change this to take std::string. Maybe this also helps working around g++ bugs we're currently facing
export inline void add_header(parser& p, const optional_string& s, int group = 0)
{
    p.add_option(option({}, 0, {}, of::none, s, group));
}

}

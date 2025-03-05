// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <argp.h>
#include <string>
#include <utility>
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
    void doc(optional_string s)
    {
        m_doc = std::move(s);
    }

    // TODO: move option into some sort of container (how?)
    // TODO: obviously this lacks the callback
    // TODO: test this
    // TODO: if we do take an universal reference here we should probably also take a normal one
    void add_option(option&& o)
    {
        // TODO: read up: why do I have to call move again here? I already have an universal refewrence, no?
        m_options.push_back(std::move(o));
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
            // TODO: this conversion from our option class to argp_option could be extracted and unit tested
            argp_options.push_back({c_str(o.name()), o.key(), c_str(o.arg()), to_int(o.flags()), c_str(o.doc()), o.group()});
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
export inline void add_header(parser& p, optional_string s, int group = 0)
{
    p.add_option(option({}, 0, {}, of::none, std::move(s), group));
}

}

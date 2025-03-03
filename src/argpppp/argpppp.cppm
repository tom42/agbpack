// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <argp.h>
#include <optional>
#include <string>
#include <utility>

// TODO: API documentation
//       * program_name (once it's settled down)

export module argpppp;

namespace argpppp
{

export std::string program_name(const char* argv0);

export class option final
{
public:
};

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
    void doc(std::optional<std::string> s)
    {
        m_doc = std::move(s);
    }

    // TODO: move option into some sort of container (how?)
    // TODO: obviously this lacks the callback
    // TODO: test this
    void add_option(option)
    {

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
        struct argp argp { nullptr, nullptr, nullptr, m_doc ? m_doc->c_str() : nullptr, nullptr, nullptr, nullptr };
        argp_parse(&argp, argc, argv, 0, nullptr, this);
    }

private:
    std::optional<std::string> m_doc;
};

}

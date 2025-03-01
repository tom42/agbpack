// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <argp.h>
#include <optional>
#include <string>

// TODO: API documentation
//       * program_name (once it's settled down)
// TODO: features to make available (go through argp examples, this one is from example 2):
//       * argp_program_version (global variable)
//       * argp_program_bug_address (global variable)
//       * doc string (goes into parser definition => want this to be dynamic!)

export module argpppp;

namespace argpppp
{

export std::string program_name(const char* argv0);

// TODO: this could use the program_name thing above too, no?
// TODO: features
//       * Does not terminate your application, unless you want it to
//       * Properly prints your program name, in any damn case
export class parser final
{
public:
    void doc(std::optional<std::string> s)
    {
        m_doc = std::move(s);
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
        struct argp argp { nullptr, nullptr, nullptr, m_doc ? m_doc->c_str() : nullptr };
        argp_parse(&argp, argc, argv, 0, nullptr, this);
    }

private:
    std::optional<std::string> m_doc;
};

}

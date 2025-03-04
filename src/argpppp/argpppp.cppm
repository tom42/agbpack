// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <argp.h>
#include <optional>
#include <string>
#include <utility>
#include <vector>

// TODO: API documentation
//       * program_name (once it's settled down)

export module argpppp;

namespace argpppp
{

export std::string program_name(const char* argv0);

export using optional_string = std::optional<std::string>;

// TODO: deinline this
// TODO: take into account that s may be empty
// TODO: unit test this
inline const char* c_str(const optional_string& s)
{
    return s ? s->c_str() : nullptr;
}

// TODO: option flags are a candidate for their own module partition?
// TODO: provide bitwise or for these, and probably also and => needs test (what would we need and for?)
// TODO: provide conversion to int => call site? (nah do a to_int(od) function and unit test that)
export enum class of
{
    none = 0,
    arg_optional = 0x1,
    hidden = 0x2,
    alias = 0x4,
    doc = 0x8,
    no_usage = 0x10
};

// TODO: add fields
//       * group
export class option final
{
public:
    // TODO: ctor and getters need testage
    option(optional_string name, int key, optional_string arg, of flags, optional_string doc, int group)
        : m_name(std::move(name))
        , m_key(key)
        , m_arg(std::move(arg))
        , m_flags(flags)
        , m_doc(doc)
        , m_group(group)
    {}

    // TODO: is this the right return type? Can we not modify both the optional and the underlying string?
    const optional_string& name() const { return m_name; }

    int key() const { return m_key; }

    const optional_string& arg() const { return m_arg; }

    of flags() const { return m_flags; }

    const optional_string& doc() const { return m_doc; }

    int group() const { return m_group; }

private:
    optional_string m_name;
    int m_key;
    optional_string m_arg;
    of m_flags;
    optional_string m_doc;
    int m_group;
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
    void doc(optional_string s)
    {
        m_doc = std::move(s);
    }

    // TODO: move option into some sort of container (how?)
    // TODO: obviously this lacks the callback
    // TODO: test this
    void add_option(option o)
    {
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

        // TODO: hardcoded options
        // TODO: turn option instances into argp_option structures. Can we unit test this? It IS testworthy, isn't it?
        // TODO: do we build op this vector here, or as options are added
        std::vector<argp_option> argp_options;
        argp_options.reserve(m_options.size());
        for (const auto& o : m_options)
        {
            // TODO: this conversion from our option class to argp_option could be extracted and unit tested
            // TODO: no cast here (flags)
            argp_options.push_back({c_str(o.name()), o.key(), c_str(o.arg()), static_cast<int>(o.flags()), c_str(o.doc()), o.group()});
        }
        argp_options.push_back({});

        struct argp argp { argp_options.data(), nullptr, nullptr, c_str(m_doc), nullptr, nullptr, nullptr };
        argp_parse(&argp, argc, argv, 0, nullptr, this);
    }

private:
    optional_string m_doc;
    std::vector<option> m_options;
};

}

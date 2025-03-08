// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <argp.h>
#include <optional>
#include <string>

export module argpppp:option;
import :of;
import :optional_string;

namespace argpppp
{

export class option final
{
public:
    option(const optional_string& name = {}, int key = 0, const optional_string& arg = {}, of flags = of::none, const optional_string& doc = {}, int group = 0)
        : m_name(name)
        , m_key(key)
        , m_arg(arg)
        , m_flags(flags)
        , m_doc(doc)
        , m_group(group)
    {}

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

// TODO: this requires strings not to go out of scope. At the very least document this!
ARGPPPP_EXPORT_FOR_UNIT_TESTING
inline struct argp_option to_argp_option(const option& o)
{
    return {c_str(o.name()), o.key(), c_str(o.arg()), to_int(o.flags()), c_str(o.doc()), o.group()};
}

}

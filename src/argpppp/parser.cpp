// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <optional>
#include <string>

module argpppp;
// TODO: do I need to import other module partitions here? Like :optional_string or :option?

namespace argpppp
{

void parser::add_option(const option& o)
{
    m_options.push_back(o);
}

void parser::doc(const optional_string& s)
{
    m_doc = s;
}

}

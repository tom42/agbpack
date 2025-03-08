// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <optional>
#include <string>

module argpppp;
// TODO: do I need to import other module partitions here? Like :optional_string?

namespace argpppp
{

void parser::doc(const optional_string& s)
{
    m_doc = s;
}

}

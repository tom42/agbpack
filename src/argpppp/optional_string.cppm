// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <optional>
#include <string>

export module argpppp:optional_string;

namespace argpppp
{

export using optional_string = std::optional<std::string>;

ARGPPPP_EXPORT_FOR_UNIT_TESTING
inline const char* c_str(const optional_string& s)
{
    return s ? s->c_str() : nullptr;
}

}

// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <string>

// TODO: API documentation
//       * program_name (once it's settled down)

export module argpppp;
export import :of;
export import :option;
export import :optional_string;
export import :parser;

namespace argpppp
{

export std::string program_name(const char* argv0);

}

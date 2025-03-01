// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <filesystem>
#include <string>

module argpppp;

namespace argpppp
{

// TODO: can we get argp to use this? would be somewhat cool, no? program name hook?
//       * well program name hook is problematic, since it would need to pass a global pointer?
//       * well we could do that easily, if we wanted, no?
// TODO: put this into its own translation unit. Rationale: keep dependencies on <filesystem> at bay.
std::string program_name(const char* argv0)
{
    return std::filesystem::path(argv0).stem().string();
}

}

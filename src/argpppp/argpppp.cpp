// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <filesystem>
#include <string>

module argpppp;

namespace argpppp
{

std::string program_name(const char* argv0)
{
    return std::filesystem::path(argv0).stem().string();
}

}

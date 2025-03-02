// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

import argpppp;

extern "C"
{
const char* argp_program_version = "argp_program_version_test 1.0";
}

int main(int argc, char** argv)
{
    argpppp::parser parser;
    parser.parse(argc, argv);
}

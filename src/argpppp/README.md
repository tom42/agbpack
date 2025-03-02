<!--
SPDX-FileCopyrightText: 2025 Thomas Mathys
SPDX-License-Identifier: MIT
-->

# argpppp

argpppp is a simple C++ wrapper around glibc's `argp_parse` function.
Its key features are:
* It comes as a C++ module
* It uses [argp-standalone](https://github.com/tom42/argp-standalone) if glibc
  is not available

# Usage

```c++
// Importing argpppp is sufficient. No need to include argp.h.
import argpppp;

// Defining argp global variables may require extern "C",
// e.g. with MSVC and argp-standalone.
extern "C" const char* argp_program_version = "1.0";

int main(int argc, char** argv)
{
    // TODO: eventually, ensure this compiles
    // TODO: doc string/resource management? Or do we keep it as a minimal example?
    argpppp::parser parser;
    parser.parse(argc, argv);
}
```
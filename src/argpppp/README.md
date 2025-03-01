<!--
SPDX-FileCopyrightText: 2025 Thomas Mathys
SPDX-License-Identifier: MIT
-->

# argpppp

argpppp is a simple C++ wrapper around glibc's `argp_parse` function.
Its key features are:
* It comes as a C++ module
* It uses [argp-standalone](https://github.com/tom42/argp-standalone) if glibc
  is not available.

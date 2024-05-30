<!--
SPDX-FileCopyrightText: 2024 Thomas Mathys
SPDX-License-Identifier: MIT
-->

# TODO
* Testing.cmake
  * Maybe we should use the local Catch2 if it is available? Should be easy on Linux
* Toplevel makefile:
  * Do we want to disable unit tests by default if not the main project?
  * Do we want to have an option to enable/disable unit tests explicitly?
* Next: we have a library with an empty module.
  * Add a bogus function to the module
  * Write a unit test that calls that function and verifies its result
* add_subdirectory(src bin)? Why would we want to do that?
* Enable warnings (which ones? Those from vtgcore, or maybe we have a look at doctest?)
* Have some sort of prefix for our own modules in cmake? I mean, Testing is a very generic name, isn't it?

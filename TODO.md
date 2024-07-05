<!--
SPDX-FileCopyrightText: 2024 Thomas Mathys
SPDX-License-Identifier: MIT
-->

# TODO
* THEN
  * Enable warnings for g++ and clang
    * see how doctest checks for compiler ID, if it looks better, use that approach
    * enable warnings from vtgcore: we already established we want these
    * see what warnings doctest has enabled, see whether these make sense
    * see what other warnings we have noted in vtgcore's TODO.md?
  * Set up our first real test (e.g. rle_decoder)
* See what other ideas from the cmake book we'd like to put into place
  * valgrind
  * code coverage for unit tests
  * clang-fmt
  * clang-tidy
  * Add reuse custom target to CMakeLists.txt?
* Checklist for a new code base
  * Automate the build => That is, have a top level shell script that runs stuff
    * configure
    * build
    * run tests
    * reuse lint
  * Turn on all error messages, treat warnings as errors
  * Set up reuse to check licensing stuff
* Toplevel makefile:
  * Do we want to disable unit tests by default if not the main project?
  * Do we want to have an option to enable/disable unit tests explicitly?
* Next: we have a library with an empty module.
  * Add a bogus function to the module
  * Write a unit test that calls that function and verifies its result
* add_subdirectory(src bin)? Why would we want to do that?
* Enable warnings (which ones? Those from vtgcore, or maybe we have a look at doctest?)
  * Well we can try /Wall with MSVC, but probably it's insane

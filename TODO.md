<!--
SPDX-FileCopyrightText: 2024 Thomas Mathys
SPDX-License-Identifier: MIT
-->

# TODO
* Wrap up testing business
  * VtgTesting.cmake
    * Do we really want to call the function enable_testing_with_catch2?
      * Thing is, that name suggests it calls enable_testing, which it does not.
      * Document that in order to use this, include(CTest) should be done, but at the top level of the file
      * Maybe we call the thing setup_catch2() (or vtg_setup_catch2)
      * And maybe we call it in the top level if BUILD_TESTING is true_
* THEN
  * Enable warnings
  * Set up our first real test (e.g. rle_decoder)
* See what other ideas from the cmake book we'd like to put into place
  * valgrind
  * code coverage for unit tests
  * clang-fmt
  * clang-tidy
* Checklist for a new code base
  * Automate the build => That is, have a top level shell script that runs stuff
    * configure
    * build
    * run tests
    * reuse lint
  * Turn on all error messages, treat warnings as errors
  * Set up reuse to check licensing stuff
* Unit test: we haven't executed add_test yet. So, if we run tests though cmake, nothing is going to happen, no?
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

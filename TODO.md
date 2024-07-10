<!--
SPDX-FileCopyrightText: 2024 Thomas Mathys
SPDX-License-Identifier: MIT
-->

# TODO
* Revisit current usage of static_cast and other casts: those we currently have
  could probably be replaced by std::to_integer.
* Data processing:
  * Maybe we define an internal type alias which we use to do all byte processing:
    * Bytes read from input get converted to that internal type
    * Bytes written to output get converted from that internal type to output type
    * => One single point of truth for conversion.
    * => The entity doing the conversion could be a template, so could adapt to input/output types
* Become reuse compliant again: our test data files break it. Use a REUSE.toml to fix
  * Note: dep5 file is deprecated, so get rid of that
* doctest has the flags below for g++ and clang. Which ones do we need too?
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
      add_compiler_flags(-Werror)
      add_compiler_flags(-fstrict-aliasing)

      # The following options are not valid when clang-cl is used.
      if(NOT MSVC)
        add_compiler_flags(-pedantic)
        add_compiler_flags(-pedantic-errors)
        add_compiler_flags(-fvisibility=hidden)
      endif()
    endif()
* vtgcore: should this also use -Wsign-conversion?
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
  * Cmake Built-in static analyzers, e.g. "include only what you need". Anything else?
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
* add_subdirectory(src bin)? Why would we want to do that?
* Enable warnings (which ones? Those from vtgcore, or maybe we have a look at doctest?)
  * Well we can try /Wall with MSVC, but probably it's insane

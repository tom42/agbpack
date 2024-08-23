<!--
SPDX-FileCopyrightText: 2024 Thomas Mathys
SPDX-License-Identifier: MIT
-->

# TODO
* Good news: huffman encoder:
  * Libgrit says this:
    * Apart from FreeImage, the LZ77 compressor and GBFS, everything is
      licenced under the MIT licence (see mit-licence.txt)
    * So that would also affect the huffman encoder. Good news, then.
      We could simply take their huffman encoder.
* Figure out whether the GBA BIOS can decode CUEs 1 and 2 bit modes.
  * If it can, then we test them in the decoder too and possibly even implement them
  * Note that GBATEK says (quote) "Data size in bit units (normally 4 or 8)"
    * It does NOT say must be 4 or 8. This might indicate that the BIOS indeed does
      support data sizes other than 4 or 8.
* Use uint8_t or std::uint8_t?
  * Same for other <cstdint> types
* clang++: reconsider the decision to use -Weverything. Maybe change that to be an option or so.
  * https://quuxplusone.github.io/blog/2018/12/06/dont-use-weverything/
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
    * enable warnings from vtgcore: we already established we want these
    * see what warnings doctest has enabled, see whether these make sense
    * see what other warnings we have noted in vtgcore's TODO.md?
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

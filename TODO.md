<!--
SPDX-FileCopyrightText: 2024 Thomas Mathys
SPDX-License-Identifier: MIT
-->

# TODO
* github workflow
  * Somehow install Catch2 so that it does not need to be built
    * On Ubuntu, use apt
    * But what about windows?
  * Get gcc build running => Works, but need a more recent g++ on github (14.2 seems to work locally)
  * Enable fail-fast (in the yml file)
* The interface for encoding and decoding is virtually the same
  * Should we give this a generic name?
    * process()
    * Function call operator
* This is somewhat unexpected:
  * A test (huffman-bitstream-alignment) indicates that the GBA BIOS does not decode data correctly
    when the uncompressed data is an odd number of bytes (not sure whether it needs to be a multiple of 2 or 4)
    * Well since the BIOS seems to be able to decode to VRAM it's to be expected that it needs some sort of alignment/padding
    * Instead of guessing around we might simply want to disassemble the damn thing.
* Maybe, just to be sure:
  * Can we prove/disprove that the bitstream needs to be aligned?
    * We would have to somehow construct an image where the huffman tree contains padding at the end
    * We would then have to somehow remove that padding and fix up the tree size field
      * A decoder that does not check bitstream alignment should then still be able to decode
      * A decoder that does check bitstream alignment should bark
      * And the ultimate interesting thing: what does a real GBA BIOS on a real GBA do?
* Maybe we should document our insights on huffman a bit
  * Copy documentation from GBATEK.txt
  * Annotate it
    * Tree size/offset (It's an offset, respectively at the end of the tree are padding bytes to align the bitstream)
      * Note: CUE encoder has the following tree size byte calculation: codetree[i] = (num_leafs - 1) | 1;
    * Alignment of bitstream (see above)
    * Encoding types (personal tests with real BIOS on emulators have shown that 1 and 2 bit is not supported, so other more exotic encodings probably aren't, either)
* Good news: huffman encoder:
  * Libgrit says this:
    * Apart from FreeImage, the LZ77 compressor and GBFS, everything is
      licenced under the MIT licence (see mit-licence.txt)
    * So that would also affect the huffman encoder. Good news, then.
      We could simply take their huffman encoder.
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
* vtgcore: should probably start using VtgEnableWarnings.cmake, since this is best we have atm
* THEN
  * Enable warnings for g++ and clang
    * See what warnings doctest has enabled for g++, see whether these make sense?
    * see what other warnings we have noted in vtgcore's TODO.md?
      * Raise warning level in gcc/clang: add -Wconversion and -Wsign-conversion?
        * Well, probably we should go ourselves through https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
          * Ugh: there is also https://gcc.gnu.org/onlinedocs/gcc/C-Dialect-Options.html
          * And: https://gcc.gnu.org/onlinedocs/gcc/C_002b_002b-Dialect-Options.html (C++)
        * Or, more detailed: https://stackoverflow.com/questions/5088460/flags-to-enable-thorough-and-verbose-g-warnings/9862800#9862800
          * This list has
            * -Wshadow
            * -Wsign-conversion
            * -Wsign-promo
            * -Wstrict-overflow=5
          * This list explicitly excludes the following, which we might want to look at anyway:
            * -Weffc++
            * -Wstrict-aliasing=3: 3 is default, be more strict?
            * -Wunsafe-loop-optimizations
* See what other ideas from the cmake book we'd like to put into place (note: this should be run on github action!)
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

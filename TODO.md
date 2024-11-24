<!--
SPDX-FileCopyrightText: 2024 Thomas Mathys
SPDX-License-Identifier: MIT
-->

# TODO
* grit source code / issues claim that an 8 bit huffman tree needs to be 512 bytes.
  * Verify one last time this is NOT true => and maybe even document this?
* Put tree serialization code into own source file
  * Attribute where it's coming from
  * Also quote the statement that leads us to believe it's MIT licensed
  * Alternatively, have all in one file, but add copyright notice to source file (copyright grit authors or something)
* Clean up tests to use new test directory thing
  * huffman_decoder_test:
    * Maybe place a readme somewhere that files have been created using reference encoders? (CUE Huffman, GBACrusher)
* How can we ensure that the library for unit testing does not get deployed/installed/whatever?
* OK: huffman tree serialization is a bitch
  * On the bright side: we're interested primarily in 4 bit coding for shrinkler-gba, so we could just call it a day :D
  * Our breadth-first traversal does not even work for a 256 byte file where each 8 bit symbol has the same frequency
  * Would a depth-first traversal work for this case? Maybe
  * Should we also write a maximum depth test?
  * Well, maybe. Prognosis:
    * breadth-first fails at even distribution, but does lucas sequence thing
    * depth-first fails at lucas sequence thing (orly? only if the short tree contains is more than one level deep?)
  * Well, we need to understand the problem.
    * I am not sure this is absolutely true, e.g. regarding number of nodes, but it sounds like it pinpoints the real problem:
      It is not the depth of the tree that matters, but the number of nodes in a subtree.
      * So we would somehow have to recognize when this happens, and kind of mix serialization of that subtree with other subtrees
        * Problem is: how to detect that case? Obviously the root node of a tree with 256 symbols always exhibits that problem
        * What if we try to do for example
          * Depth first
          * But whenever we reached a depth too deep, we somehow backtrack and do another bit of the tree?
            * Well maybe, but
              * How far do we backtrack?
              * Which path do we take then?
              * How do we get back onto the path we backtracked from?
              * How do we know we processed all the nodes?
* Need to write lots of huffman tests now (ugh.. what we have already does not work...)
* HUFFMAN.md
  * Some section on the overall format, just for the sake of completeness
* huffman_encoder implementation
  * Huffman encoder tests:
    * Maybe abandon comparing against CUE altogether: there is little value in doing so
      * Compare against decoder only. But maybe also test expected size
      * And improve some tests:
        * Like have 1 symbol, 1 time, but also 1 symbol, 32 times, 33 times
  * echo -n ABBCCDDEEEE>foo
    * When encoded with CUE huffman, this seems to create
      an odd tree which contains 'A' twice.
      * I may have hallucinated this, but maybe investigate
      * Why is that?
        * But the code for both instances of 'A' is the same?
      * What does our tree generation/serializaton code?
  * Next:
    * Do not forget: for encoder testing we cannot use CUE input!
      * We should mark all files generated with CUE as such!
      * Or well at least we should it note down somewhere
* There is a test for the RLE decoder which tests decoding through a file stream
  * This is confusing, and what we really want is probably such a test for each encoder/decoder
    * Just because it works with the RLE decoder doesn't automatically mean it works with all other decoders/encoders, no?
  * Input data too big tests:
    * Do we need one for each decoder? These tests are slow on MSVC debug builds
* Encoders:
  * These use all byte_reader to read input data.
  * This is not wrong, but byte_reader.read8() throws decode_exception if eof is reached
  * That's fine, since the encoders are written such that this cannot happen
  * Still, it's conceptually ugly/wrong. Need to do something about this (?)
* Overflow with files > 4GB
  * Affects byte_reader/byte_writer/unbounded_byte_writer
  * These have m_nbytes_read and m_nbytes_written variables, which are 32 bit
  * If we attempt to process a file > 4GB, then things will go wrong
* github workflow
  * Somehow install Catch2 so that it does not need to be built
    * On Ubuntu, use apt
    * But what about windows?
  * Get gcc build running => Works, but need a more recent g++ on github (14.2 seems to work locally)
  * Enable fail-fast (in the yml file)
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
    * Alignment of bitstream (see above)
    * Encoding types (personal tests with real BIOS on emulators have shown that 1 and 2 bit is not supported, so other more exotic encodings probably aren't, either)
* Use uint8_t or std::uint8_t?
  * Same for other <cstdint> types
* vtgcore: should probably start using VtgEnableWarnings.cmake, since this is best we have atm
* THEN
  * Enable warnings for g++ and clang
    * See what warnings doctest has enabled for g++, see whether these make sense? (see snippet below)
    * see what other warnings we have noted in vtgcore's TODO.md?
      * Raise warning level in gcc/clang: add -Wconversion?
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
  * We should build agbpack_unit_testing only if testing is enabled

---8<--- doctest common.cmake ---

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compiler_flags(-fstrict-aliasing)

    # The following options are not valid when clang-cl is used.
    if(NOT MSVC)
        add_compiler_flags(-pedantic)
        add_compiler_flags(-pedantic-errors)
        add_compiler_flags(-fvisibility=hidden)
    endif()
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    add_compiler_flags(-fdiagnostics-show-option)
    add_compiler_flags(-Wconversion)
    add_compiler_flags(-Wfloat-equal)
    add_compiler_flags(-Wlogical-op)
    add_compiler_flags(-Wredundant-decls)
    add_compiler_flags(-Wstrict-overflow=5)
    add_compiler_flags(-Wnon-virtual-dtor)
    add_compiler_flags(-Woverloaded-virtual)

    # add_compiler_flags(-Wsuggest-final-methods)
    # add_compiler_flags(-Wsuggest-final-types)

    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0)
        add_compiler_flags(-Wno-missing-field-initializers)
    endif()

    if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0)
        add_compiler_flags(-Wdouble-promotion)
        add_compiler_flags(-Wtrampolines)
        add_compiler_flags(-Wvector-operation-performance)
    endif()

    if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 6.0)
        add_compiler_flags(-Wshift-overflow=2)
    endif()

    if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.0)
        add_compiler_flags(-Walloc-zero)
    endif()
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compiler_flags(-Qunused-arguments -fcolor-diagnostics) # needed for ccache integration
endif()

if(MSVC)
    add_compiler_flags(/std:c++latest) # for post c++14 updates in MSVC
    add_compiler_flags(/permissive-)   # force standard conformance - this is the better flag than /Za
    add_compiler_flags(/WX)
    add_compiler_flags(/Wall) # turns on warnings from levels 1 through 4 which are off by default - https://msdn.microsoft.com/en-us/library/23k5d385.aspx

    add_compiler_flags(
        /wd4514 # unreferenced inline function has been removed
        /wd4571 # SEH related
        /wd5264 # const variable is not used
        /wd4710 # function not inlined
        /wd4711 # function 'x' selected for automatic inline expansion

        /wd4616 # invalid compiler warnings - https://msdn.microsoft.com/en-us/library/t7ab6xtd.aspx
        /wd4619 # invalid compiler warnings - https://msdn.microsoft.com/en-us/library/tacee08d.aspx

        #/wd4820 # padding in structs
        #/wd4625 # copy constructor was implicitly defined as deleted
        #/wd4626 # assignment operator was implicitly defined as deleted
        #/wd5027 # move assignment operator was implicitly defined as deleted
        #/wd5026 # move constructor was implicitly defined as deleted
        #/wd4623 # default constructor was implicitly defined as deleted
    )
endif()

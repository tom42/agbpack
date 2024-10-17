<!--
SPDX-FileCopyrightText: 2024 Thomas Mathys
SPDX-License-Identifier: MIT
-->

-------------------------------------------------------------
--- CreateTree --------------------------------
s:041 w:000001 n:000 d:008 l:--- r:--- nl:001
s:042 w:000002 n:001 d:008 l:--- r:--- nl:001
s:043 w:000002 n:002 d:009 l:--- r:--- nl:001
s:044 w:000002 n:003 d:009 l:--- r:--- nl:001
s:045 w:000002 n:004 d:00A l:--- r:--- nl:001
s:046 w:000002 n:005 d:00A l:--- r:--- nl:001
s:047 w:000003 n:006 d:00B l:--- r:--- nl:001
s:048 w:000004 n:007 d:00C l:--- r:--- nl:001
s:101 w:000003 n:008 d:00B l:000 r:001 nl:002
s:102 w:000004 n:009 d:00C l:002 r:003 nl:002
s:103 w:000004 n:00A d:00D l:004 r:005 nl:002
s:104 w:000006 n:00B d:00D l:006 r:008 nl:003
s:105 w:000008 n:00C d:00E l:007 r:009 nl:003
s:106 w:00000A n:00D d:00E l:00A r:00B nl:005
s:107 w:000012 n:00E d:--- l:00C r:00D nl:008

--- CreateCodeTree ----------------------------
N:000 L:07 R:00 XX|008 --|001
N:001 L:00 R:01 L-|002 --|003
N:002 s:08 R:01 ------ LR|004
N:003 L:01 R:02 LR|005 L-|006
N:004 s:03 s:04 ------ ------
N:005 s:05 s:06 ------ ------
N:006 s:07 R:00 ------ LR|007
N:007 s:01 s:02 ------ ------

--- CreateCode --------------------------------
28 12 00 00
(+CodeTree)
92 9B F4 EF 00 00 B0 DD 
-------------------------------------------------------------

# TODO
* huffman_encoder
  * echo -n ABBCCDDEEEE>foo
    * When encoded with CUE huffman, this seems to create
      an odd tree which contains 'A' twice.
      * I may have hallucinated this, but maybe investigate
      * Why is that?
        * But the code for both instances of 'A' is the same?
      * What does our tree generation/serializaton code?
  * Next:
    * Start writing that tree serialization code
      * Understand the serialized format
      * Implement serializer (and do not forget those max. offsets!)
    * Set up a special test where we (=> repurposed zzz_test for this)
      * Serialize a bitstream using our own code
        * Is in place, although only for 8 bit huffman and it does not yet flush the bitbuffer
    * Tree serialization
      * Here we should have a couple of sanity checks:
        * Maximum tree size
        * Maximum child node offset
        * Question: assert or exception?
          * Probably exception: we're rather unsure how to do it correctly,
            so for starters we probably want it also in release builds.
    * Bitstream encoding
      * We could implement that one first:
        * We create the concept of a code table. This contains code+length for each symbol
        * When we generate the bitstream, all we need is such a code table
        * We can then go and first use output from CUE
          * We need the serialized tree (can get that from a CUE file and hardcode it)
          * From the serialized tree we can also generate a code table
            * That is, we can have code that generates a code table from
              * A huffman_decoder_tree
              * Our own encoder tree
    * Do not forget: for encoder testing we cannot use CUE input!
      * We should mark all files generated with CUE as such!
      * Or well at least we should it note down somewhere
  * We can now go and dump codes.
    * First of all, see whether we can somehow dump the codes from the decoder_tree
    * Write a recursive dump function
    * To debug it, compare it against output printed during decoding
  * Can we do the following experiment:
    * We take some reference data and try to dump all the huffman codes from the GBA tree
    * We take that reference data, let our huffman tree code process it and dump
      all the codes too: do we get the same output of codes? If not we don't even need
      to continue!
    * BEFORE we do this:
      * We should follow the naming that GBATEK uses, which is 0/1
        * Only now we can dump codes and see whether we get LSB/MSB order of codes right
    * Once all this is settled we can worry about
      * Serializing the tree on one hand
        * Here we need to fully understand the serialization of the tree
          * Maybe we need to encode some data using a reference encoder and then
            manually analyze/annotate the resulting serialized tree
      * And encoding the data on the other hand
  * Once all of this works, see how we get our code built with g++ again
    * Well possibly occasionally build it with -Wno-attributes, so that we don't
      have to fix too much stuff once we get back to g++.
* g++ currently requires -Wno-attributes, but that's due to a bug in its library headers,
  as I understand. Not sure how to go about this. Probably we'll have to temporarily
  suppress this warning, so that we can keep testing with g++.
* There is a test for the RLE decoder which tests decoding through a file stream
  * This is confusing, and what we really want is probably such a test for each encoder/decoder
    * Just because it works with the RLE decoder doesn't automatically mean it works with all other decoders/encoders, no?
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

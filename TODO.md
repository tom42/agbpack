<!--
SPDX-FileCopyrightText: 2024 Thomas Mathys
SPDX-License-Identifier: MIT
-->

# TODO
* testdata
  * Maybe using CC0-1.0 as test data is not a good idea:
    * There is a REUSE.toml file that says that all testdata files are copyright mine
    * For some reason, REUSE lint on WSL2 does flag the CC0-1.0 test data file
    * Probably it's best to replace it, e.g. by our HUFFMAN.md file or something
* LZSS: implement optimal backward parse
* Do we need some test suite for GBA?
  * Those huffman tests we did earlier
  * Some lzss decoder tests, certainly at least that one: aaaaaaaaaaaaaaaaaaa
  * Problem:
    * We need an emulator to sensibly do this with
    * And we also need a way to crunch our stuff, although we could use a script for this
    * Well maybe not, then?
* Start using import std; ?
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
* HUFFMAN.md
  * Some section on the overall format, just for the sake of completeness
* huffman_encoder implementation
  * Huffman encoder tests:
    * Maybe abandon comparing against CUE altogether: there is little value in doing so
      * Compare against decoder only. But maybe also test expected size
      * And improve some tests:
        * Like have 1 symbol, 1 time, but also 1 symbol, 32 times, 33 times
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
  * Also build 32 bit targets? (And if so, with which compilers?)
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

<!--
SPDX-FileCopyrightText: 2024 Thomas Mathys
SPDX-License-Identifier: MIT
-->

# TODO
* Redo encoders/decoders
  * Forget about supporting non-random access iterators where not feasible - it's just plain pointless
  * This means, in particular, that we also remove specializations for random access iterators we did for the LZSS encoder
* Reimplement optimal_lzss_encoder
  * Added clownlzss sources
    * Plan:
      * Integrate clownlzss.c as part of agbpack => In progress, but need to compile it separately as an object library due to warnings
        (we can't compile it as part of the remaining agbpack sources - their warning level is simply too high)
      * Wenn we do so, can we avoid having to install clownlzss.h with the module?
        * Or is this even a problem?
  * Once we have this we could measure: if it performs faster than the existing encoder, then there is
    no reason to have the existing encoder
* Honor all the options added to CMakeLists.txt
  * Only get argpppp if tools are used
* Turn our debug tools into real tools in src, with proper argp_parse command line parser
  * agbpack-tool / agbpacker: simple compression/decompression tool exposing all implemented schemes
    * Rationale: will be useful as a tool in itself
    * Also useful for comparison with other GBA LZSS implementations
    * We have something to tinker around with C++ argp_parse wrappers
  * agbpack_test_tools_common: this is not used anymore. See what bits we can use to implement
    agbpacker, delete the rest.
* Consider switching gears?
  * Why not simply use clownlzss? It gives us what we want: optimal LZSS encoding
  * So what if we incorporate clownlzss into agbpack?
    * We could then go for an agbpack release
    * That in turn means we can go back to shrinkler-gba and a release of that
    * And that in turn means we can release an 1k and a 4k prod for the GBA
      * Would that not be awesome?
* Do the data types in common.cppm even make sense (agbpack_io_datatype, agbpack_u8, agbpack_u16, agbpack_u32)?
  * agbpack_io_datatype: possibly, apart from the fact that it is not used consistently
  * agbpack_u8, agbpack_u16, agbpack_u32: probably not, we could probaby just as well use fixed size standard types (uint8_t etc)
* Redesign exception handling
  * We REALLY should revise exception handling, otherwise we're going to end up with a big mess
  * We have no real need for decode_exception and encode_exception, no? (Well, they don't hurt, either, and are no big deal to keep)
  * However, if we threw them out and had only agbpack_exception, then obviously that would solve the problem of having an encoder that throws decode_exception
    * Not sure yet we're going that route, though.
      * What about exception messages?
    * Most of them are probably OK, although very generic
    * The one thrown by delta16 encoding hwever is wrong: "encoded data is corrupt", but the real problem is that the input data has an odd length
      * It could be more generic: "could not encode data: premature end of input"
      * Or more accurate: "could not encode data: input does not contain an even number of bytes"
    * Maybe lower layers just use more generic exception messsages, and the encode/ecode functions add a message prefix:
      * Decoder: "could not decode data: premature end of file"
      * Encoder: "could not encode data: input contains an odd number of bytes"
      * Either way: we need to test this, and then we're good and can easily change stuff and notice when we screw up things. The usual drill, really.
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
  * Somehow install Catch2 so that it does not need to be built (Note: this is probably when our build matrix explodes and we really should have two workflows, one for linux, one for Windows)
    * On Ubuntu, use apt
    * But what about windows? (Well we can always fall back to our Fetch_Content solution)
* This is somewhat unexpected:
  * A test (huffman-bitstream-alignment) indicates that the GBA BIOS does not decode data correctly
    when the uncompressed data is an odd number of bytes (not sure whether it needs to be a multiple of 2 or 4)
    * Well since the BIOS seems to be able to decode to VRAM it's to be expected that it needs some sort of alignment/padding
    * Instead of guessing around we might simply want to disassemble the damn thing.
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

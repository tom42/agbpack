<!--
SPDX-FileCopyrightText: 2024 Thomas Mathys
SPDX-License-Identifier: MIT
-->

# TODO
* Add install target(s)
  * We keep writing a mini project for testing the consumation of an installed
    library => Maybe write that project one last time and put it under version control?
  * Add a new install target for agbpack (the static library)
    * Big question here: Do we have to install clownlzss.h along with the module since clownlzss.h is used in a .cppm file?
      * We could install the clownlzss.h header, problem is, it's got compiler flags attached to it (/Zc:__cplusplus),
        so we have the same problem we had with warning flags.
      * Well for starters see whether it is a problem at all and whether we can get
        rid of it using generator expressions:
        * For testing, set up an experiment where we have a gcc specific option in there
        * Can compile that library with gcc and install it?
        * Can we then consume that library with clang without clang choking on
          a gcc specific option that somehow slipped in?
* Redo encoders/decoders
  * Forget about supporting non-random access iterators where not feasible - it's just plain pointless
  * This means, in particular, that we also remove specializations for random access iterators we did for the LZSS encoder
* Turn our debug tools into real tools in src, with proper argp_parse command line parser
  * agbpack-tool / agbpacker: simple compression/decompression tool exposing all implemented schemes
    * Rationale: will be useful as a tool in itself
    * Also useful for comparison with other GBA LZSS implementations
    * We have something to tinker around with C++ argp_parse wrappers
  * agbpack_test_tools_common: this is not used anymore. See what bits we can use to implement
    agbpacker, delete the rest.
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
    * Maybe lower layers just use more generic exception messages, and the encode/ecode functions add a message prefix:
      * Decoder: "could not decode data: premature end of file"
      * Encoder: "could not encode data: input contains an odd number of bytes"
      * Either way: we need to test this, and then we're good and can easily change stuff and notice when we screw up things. The usual drill, really.
* Do we need some test suite for GBA?
  * Those huffman tests we did earlier
  * Some lzss decoder tests, certainly at least that one: aaaaaaaaaaaaaaaaaaa
  * Problem:
    * We need an emulator to sensibly do this with
    * And we also need a way to crunch our stuff, although we could use a script for this (well we're going to have agbpacker, no?)
    * Well maybe not, then?
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
  * https://github.com/include-what-you-use: unfortunately this segfaults
* Checklist for a new code base
  * Automate the build => That is, have a top level shell script that runs stuff
    * configure
    * build
    * run tests
    * reuse lint

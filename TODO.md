<!--
SPDX-FileCopyrightText: 2024 Thomas Mathys
SPDX-License-Identifier: MIT
-->

# TODO
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
* How can we ensure that the library for unit testing does not get deployed/installed/whatever?
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
* Maybe, just to be sure:
  * Can we prove/disprove that the bitstream needs to be aligned?
    * We would have to somehow construct an image where the huffman tree contains padding at the end
    * We would then have to somehow remove that padding and fix up the tree size field
      * A decoder that does not check bitstream alignment should then still be able to decode
      * A decoder that does check bitstream alignment should bark
      * And the ultimate interesting thing: what does a real GBA BIOS on a real GBA do?
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

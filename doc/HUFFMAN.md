<!--
SPDX-FileCopyrightText: 2024 Thomas Mathys
SPDX-License-Identifier: MIT
-->

# Huffman coding in the GBA BIOS

This document contains some notes on the huffman coding scheme
implemented the GBA BIOS.

## Terminology

Some terminology is borrowed from ARM CPUs.
Moreover, little endianness is assumed everywhere.

|Term         |Description                                            |
|-------------|-------------------------------------------------------|
|Byte         |An 8 bit quantity                                      |
|Halfword     |A 16 bit quantity                                      |
|Word         |A 32 bit quantity                                      |
|Node0, Child0|Child node to follow when there is a 0 in the bitstream|
|Node1, Child1|Child node to follow when there is a 1 in the bitstream|

## Huffman tree serialization

The serialized huffman tree is best viewed as an array of halfwords,
where each array element holds two sibling nodes.

Since each sibling node pair is stored in a halfword, each node is
stored in a byte. So a leaf node has 8 bits to store the symbol
it represents. With 8 bit symbols this leaves no room in a node to
store the node's type. That information is stored in the parent node.

The array element at index 0 is special: rather than a pair of
sibling nodes it stores the size of the serialized tree and
the root node. The root node is implicitly assumed to be an internal
node.

### Tree size

As mentioned, the first halfword of the serialized huffman tree
contains the tree size byte, let's call it TSB. It is an unsigned
8 bit value. The tree size in bytes is given by 2 * (TSB + 1).

The maximum tree size is therefore 2 * (255 + 1) = 512 bytes.

A huffman tree for N symbols always contains 2 * N - 1 nodes.

For 8 bit wide symbols, N=256, which yields 2 * 256 - 1 = 511 nodes.
Since each node occupies one byte, the maximum tree size of
512 bytes is just enough to hold a tree for 256 symbols plus the
tree size byte.

After the serialized tree the encoded bitstream follows.
The encoded bitstream must be word aligned. This means that padding
bytes may be required between the serialized tree and the encoded
bitstream. These padding bytes must be contained in the size
specified by the TSB. In other words, the tree size in bytes must be
a multiple of 4 bytes and the TSB should always have an odd value,
and the last 2 bytes of a serialized tree may be unused.

Note that the GBA BIOS relies on the bitstream being word aligned.
It reads the bitstream a word at a time. If the bitstream is
misaligned, the decoder, running on an ARM CPU, will produce garbage.

### Format of internal nodes

|Bit 7      |Bit 6      |Bits 0 - 5        |
|-----------|-----------|------------------|
|Child0 type|Child1 type|Offset to children|

Bits 6 and 7 store the type of the internal node's child nodes,
where 0 means the respective child is an internal node and 1 means
it is a leaf node.

GBATEK describes the offset as follows:

```
Offset to next child node,
Next child node0 is at (CurrentAddr AND NOT 1)+Offset*2+2
Next child node1 is at (CurrentAddr AND NOT 1)+Offset*2+2+1
```

This is totally correct, but a rather complicated way of looking at
things. It results from treating the serialized tree as a lump of
bytes and doing all byte addressing on it.

A simpler way of looking at things is to treat the serialized tree as
an array of halfwords, where each halfword holds two sibling nodes.
The offset can then be interpreted as the number of array elements
plus one to get from the current array element to the next one.
This may not be the most practical way to implement a decoder,
but it's a good way to understand the format of the serialized tree.

The fact that the offset is stored in only 6 bits makes serializing
the huffman tree somewhat difficult: even though it is an index into
an array of halfwords, 6 bits are obviously not enough to cover all
of the 511 nodes that a huffman tree for 256 symbols has.

This means that a naive implementation of tree serialization that
does simple depth-first or breadth-first traversal of the tree may
fail for some inputs because it produces overflows in the offset
field of some internal nodes.

### Format of leaf nodes

|Bits 0 - 7                             |
|---------------------------------------|
|The symbol represented by the leaf node|

When the symbol size is less than 8 bits, the upper unused bits must
be zero.

### Example

Note: the data for the following example has been produced with CUE's
huffman encoder: https://github.com/PeterLemon/Nintendo_DS_Compressors

Consider the following message: ABBCCDDEEFFGGGHHHH

The following table shows the symbol frequencies and the resulting
huffman code for the message:

|Symbol  |Frequency|Code|Length|
|--------|---------|----|------|
|A (0x41)|1        |1110|4     |
|B (0x42)|2        |1111|4     |
|C (0x43)|2        |010 |3     |
|D (0x44)|2        |011 |3     |
|E (0x45)|2        |100 |3     |
|F (0x46)|2        |101 |3     |
|G (0x47)|3        |110 |3     |
|H (0x48)|4        |00  |2     |

The following hex dump shows the entire compressed file:

```
Address     Data           Description
---------------------------------------------
00000000    28 12 00 00    Compression header
00000004    07 00 80 01    Serialized tree
00000010    48 c1 c1 82    Serialized tree
00000014    43 44 45 46    Serialized tree
00000020    47 c0 41 42    Serialized tree
00000024    92 9b f4 ef    Bitstream
00000030    00 00 b0 dd    Bitstream
```

The following table tries to explain the serialized tree.
Each row is a halfword containing two sibling nodes.
Note that child0 is the LSB of the halfword, child1 the MSB.

```
Index  Child0 (Bit 0-7)      Child1 (Bit 8-15)
----------------------------------------------
0      0x07 TSB              0x00
1      0x80                  0x01
2      0x48 'H'              0xc1
3      0xc1                  0x82
4      0x43 'C'              0x44 'D'
5      0x45 'E'              0x46 'F'
6      0x47 'G'              0xc0
7      0x41 'A'              0x42 'B'
```

The tree size in bytes is 2 * (TSB + 1) = 2 * (7 + 1) = 16 bytes.
There are 8 child nodes representing the letters A to H.
A huffman tree with 8 child nodes has 2 * 8 - 1 = 15 nodes in total.
Another byte is needed to store the TSB, so we end up with 16 bytes,
so there are no padding bytes at the end of the serialized tree.

Assume the decoder has just decoded a symbol and encounters in
the middle of the bitstream the following sequence of bits: 101

1. The decoder starts at current array index I=0 with the root node,
   which is implicitly an internal node. Note that child0 type and
   child1 type are both 0, so both child nodes are internal nodes too.
   Offset Ofs is also 0. The decoder reads now the next bit from the
   bitstream. The bit is 1, which means that the decoder needs to
   move to the child1 node. The decoder can now move to the next
   array index: I = I + Ofs + 1 = 0 + 0 + 1 = 1
2. The decoder is at I=1, looking at child1 because the bit just
   read was 1. The node's value is 0x01, so child0 type and child1
   type are again 0, so both are again internal nodes. Ofs is 1.
   The decoder reads the next bit from the bitstream. The bit is 0,
   so the decoder will move to the child0 node. The decoder can now
   move to the next array index: I = I + Ofs + 1 = 1 + 1 + 1 = 3
3. The decoder is at I=3, looking at child0 because the bit just
   read was 0. The node's value is 0xc1 = 0b11000001, so child0 type
   and child1 type are both 1, so both are leaf nodes. Ofs is 1.
   The decoder reads the next bit from the bitstream. The bit is 1,
   so the decoder will move to the child1 node. The decoder can now
   move to the next array index: I = I + Ofs + 1 = 3 + 1 + 1 = 5
4. The decoder is at I=5, looking at child1 because the bit just
   read was 1. The node is a leaf node, so the node's value 0x46 is
   the symbol 'F'. We're done.

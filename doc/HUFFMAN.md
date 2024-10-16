<!--
SPDX-FileCopyrightText: 2024 Thomas Mathys
SPDX-License-Identifier: MIT
-->

# Huffman encoding/decoding in the GBA BIOS

This document contains some notes on the huffman encoding scheme
implemented the GBA BIOS.

## Terminology

Terminology is borrowed from ARM CPUs.
Moreover, little endianness is assumed everywhere.

|Term         |Description                                                               |
|-------------|--------------------------------------------------------------------------|
|Byte         |An 8 bit quantity                                                         |
|Halfword     |A 16 bit quantity                                                         |
|Word         |A 32 bit quantity                                                         |
|node0, child0|The child node the decoder follows when it encounters a 0 in the bitstream|
|node1, child1|The child node the decoder follows when it encounters a 1 in the bitstream|

## Huffman tree serialization

The serialized huffman tree is best viewed as an array of halfwords,
where each array element holds two sibling nodes.

The array element at index 0 is special: rather than a pair of
sibling nodes it stores the size of the serialized tree and
the root node.

Since each sibling node pair is stored in a halfword, each node is
stored in a byte. So a leaf node has 8 bits to store the symbol
it represents. With 8 bit symbols this leaves no room in a node to
store the node's type. That information is stored in the parent node.
The root node is implicitly assumed to be an intermediate node.

### Format of intermediate nodes

TODO: describe it (What about the offset?)

### Format of leaf nodes

TODO: describe it (what about upper unused bits?)

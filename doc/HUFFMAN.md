<!--
SPDX-FileCopyrightText: 2024 Thomas Mathys
SPDX-License-Identifier: MIT
-->

# Huffman coding in the GBA BIOS

This document contains some notes on the huffman coding scheme
implemented the GBA BIOS.

## Terminology

Terminology is borrowed from ARM CPUs.
Moreover, little endianness is assumed everywhere.

|Term         |Description                                            |
|-------------|-------------------------------------------------------|
|Byte         |An 8 bit quantity                                      |
|Halfword     |A 16 bit quantity                                      |
|Word         |A 32 bit quantity                                      |
|node0, child0|Child node to follow when there is a 0 in the bitstream|
|node1, child1|Child node to follow when there is a 1 in the bitstream|

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

### Format of internal nodes

TODO: describe it (What about the offset?)

|Bit 7      |Bit 6      |Bits 0 - 5        |
|-----------|-----------|------------------|
|child0 type|child1 type|Offset to children|

Bits 6 and 7 store the type of the internal node's child nodes,
where 0 means the respective child is an internal node and 1 means
it is a leaf node.

### Format of leaf nodes

|Bits 0 - 7                             |
|---------------------------------------|
|The symbol represented by the leaf node|

When the symbol size is less than 8 bits, the upper unused bits must
be zero.

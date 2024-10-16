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

|Term    |Description      |
|--------|-----------------|
|Byte    |An 8 bit quantity|
|Halfword|A 16 bit quantity|
|Word    |A 32 bit quantity|

## Huffman tree serialization

The serialized huffman tree is best viewed as an array of halfwords,
where each array element holds two sibling nodes of the tree.

In a huffman tree, sibling nodes are always both either internal
nodes or leaf nodes.

The array element at index 0 is special: rather than a pair of
sibling nodes it stores the size of the serialized tree and
the root node, which is always an internal node.

# TODO: siblings are always either internal nodes or leaf nodes
#       => document how this is relevant for the serialized tree.
# TODO: document specialty of the root node

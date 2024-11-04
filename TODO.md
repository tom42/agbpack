<!--
SPDX-FileCopyrightText: 2024 Thomas Mathys
SPDX-License-Identifier: MIT
-->

================================================================================================================
#include "cprs.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <memory>
#include <tuple>
#include <vector>

namespace
{
/** @brief Huffman node */
class Node
{
public:
	/** @brief Parameterized constructor
	 *  @param val   Node value
	 *  @count count Node count
	 */
	Node (uint8_t val, size_t count) : count (count), val (val)
	{
	}

	/** @brief Parameterized constructor
	 *  @param left  Left child
	 *  @count right Right child
	 */
	Node (std::unique_ptr<Node> left, std::unique_ptr<Node> right)
	    : child{std::move (left), std::move (right)}, count (child[0]->count + child[1]->count)
	{
		// set children's parent to self
		child[0]->parent = this;
		child[1]->parent = this;
	}

	Node () = delete;

	Node (const Node &other) = delete;

	Node (Node &&other) = delete;

	Node &operator= (const Node &other) = delete;

	Node &operator= (Node &&other) = delete;

	/** @brief Comparison operator
	 *  @param other Object to compare
	 */
	bool operator< (const Node &other) const
	{
		// major key is count
		if (count != other.count)
			return count < other.count;

		// minor key is value
		return val < other.val;
	}

	/** @brief Whether this node is a parent */
	bool isParent () const
	{
		return static_cast<bool> (child[0]);
	}

	/** @brief Build Huffman codes
	 *  @param[in] node    Huffman node
	 *  @param[in] code    Huffman code
	 *  @param[in] codeLen Huffman code length (bits)
	 */
	static void buildCodes (std::unique_ptr<Node> &node, uint32_t code, size_t codeLen);

	/** @brief Build lookup table
	 *  @param[in] nodes Table to fill
	 *  @param[in] n     Huffman node
	 */
	static void buildLookup (std::vector<Node *> &nodes, const std::unique_ptr<Node> &node);

	/** @brief Serialize Huffman tree
	 *  @param[out] tree Serialized tree
	 *  @param[in]  node Root of subtree
	 *  @param[in]  next Next available slot in tree
	 */
	static void serializeTree (std::vector<Node *> &tree, Node *node, unsigned next);

	/** @brief Fixup serialized Huffman tree
	 *  @param[inout] tree Serialized tree
	 */
	static void fixupTree (std::vector<Node *> &tree);

	/** @brief Encode Huffman tree
	 *  @param[out] tree Huffman tree
	 *  @param[in]  node Huffman node
	 */
	static void encodeTree (std::vector<uint8_t> &tree, Node *node);

	/** @brief Count number of nodes in subtree
	 *  @returns Number of nodes in subtree
	 */
	size_t numNodes () const
	{
		// sum of children plus self
		if (isParent ())
			return child[0]->numNodes () + child[1]->numNodes () + 1;

		// this is a data node, just count self
		return 1;
	}

	/** @brief Count number of leaves in subtree
	 *  @returns Number of leaves in subtree
	 */
	size_t numLeaves ()
	{
		if (leaves == 0)
		{
			if (isParent ())
			{
				// sum of children
				leaves = child[0]->numLeaves () + child[1]->numLeaves ();
			}
			else
			{
				// this is a data node; it is a leaf
				leaves = 1;
			}
		}

		return leaves;
	}

	/** @brief Get code */
	uint32_t getCode () const
	{
		assert (!isParent ());
		return code;
	}

	/** @brief Get code length */
	uint8_t getCodeLen () const
	{
		assert (!isParent ());
		return codeLen;
	}

private:
	Node *parent;                   ///< Parent node
	std::unique_ptr<Node> child[2]; ///< Children nodes
	size_t count    = 0;            ///< Node weight
	uint32_t code   = 0;            ///< Huffman encoding
	unsigned leaves = 0;            ///< Number of leaves
	uint8_t val     = 0;            ///< Huffman tree value
	uint8_t codeLen = 0;            ///< Huffman code length (bits)
#ifndef NDEBUG
	uint16_t pos = 0; ///< Huffman tree position
#endif
};

void Node::buildCodes (std::unique_ptr<Node> &node, uint32_t code, size_t codeLen)
{
	// don't exceed 32-bit codes
	assert (codeLen < 32);

	if (node->isParent ())
	{
		// build codes for each subtree
		assert (node->child[0] && node->child[1]);
		buildCodes (node->child[0], (code << 1) | 0, codeLen + 1);
		buildCodes (node->child[1], (code << 1) | 1, codeLen + 1);
	}
	else
	{
		// set code for data node
		assert (!node->child[0] && !node->child[1]);
		node->code    = code;
		node->codeLen = codeLen;
	}
}

void Node::buildLookup (std::vector<Node *> &nodes, const std::unique_ptr<Node> &node)
{
	if (!node->isParent ())
	{
		// set lookup entry
		nodes[node->val] = node.get ();
		return;
	}

	// build subtree lookups
	buildLookup (nodes, node->child[0]);
	buildLookup (nodes, node->child[1]);
}

void Node::serializeTree (std::vector<Node *> &tree, Node *node, unsigned next)
{
	assert (node->isParent ());

	if (node->numLeaves () > 0x40)
	{
		// this subtree will overflow the offset field if inserted naively
		tree[next + 0] = node->child[0].get ();
		tree[next + 1] = node->child[1].get ();

		unsigned a = 0;
		unsigned b = 1;

		if (node->child[1]->numLeaves () < node->child[0]->numLeaves ())
			std::swap (a, b);

		if (node->child[a]->isParent ())
		{
			node->child[a]->val = 0;
			serializeTree (tree, node->child[a].get (), next + 2);
		}

		if (node->child[b]->isParent ())
		{
			node->child[b]->val = node->child[a]->numLeaves () - 1;
			serializeTree (tree, node->child[b].get (), next + 2 * node->child[a]->numLeaves ());
		}

		return;
	}

	std::deque<Node *> queue;

	queue.emplace_back (node->child[0].get ());
	queue.emplace_back (node->child[1].get ());

	while (!queue.empty ())
	{
		node = queue.front ();
		queue.pop_front ();

		tree[next++] = node;

		if (!node->isParent ())
			continue;

		node->val = queue.size () / 2;

		queue.emplace_back (node->child[0].get ());
		queue.emplace_back (node->child[1].get ());
	}
}

void Node::fixupTree (std::vector<Node *> &tree)
{
	for (unsigned i = 1; i < tree.size (); ++i)
	{
		if (!tree[i]->isParent () || tree[i]->val <= 0x3F)
			continue;

		unsigned shift = tree[i]->val - 0x3F;

		if ((i & 1) && tree[i - 1]->val == 0x3F)
		{
			// right child, and left sibling would overflow if we shifted;
			// shift the left child by 1 instead
			--i;
			shift = 1;
		}

		unsigned nodeEnd   = i / 2 + 1 + tree[i]->val;
		unsigned nodeBegin = nodeEnd - shift;

		unsigned shiftBegin = 2 * nodeBegin;
		unsigned shiftEnd   = 2 * nodeEnd;

		// move last child pair to front
		auto tmp = std::make_pair (tree[shiftEnd], tree[shiftEnd + 1]);
		std::memmove (
		    &tree[shiftBegin + 2], &tree[shiftBegin], sizeof (Node *) * (shiftEnd - shiftBegin));
		std::tie (tree[shiftBegin], tree[shiftBegin + 1]) = tmp;

		// adjust offsets
		tree[i]->val -= shift;
		for (unsigned index = i + 1; index < shiftBegin; ++index)
		{
			if (!tree[index]->isParent ())
				continue;

			unsigned node = index / 2 + 1 + tree[index]->val;
			if (node >= nodeBegin && node < nodeEnd)
				++tree[index]->val;
		}

		if (tree[shiftBegin + 0]->isParent ())
			tree[shiftBegin + 0]->val += shift;
		if (tree[shiftBegin + 1]->isParent ())
			tree[shiftBegin + 1]->val += shift;

		for (unsigned index = shiftBegin + 2; index < shiftEnd + 2; ++index)
		{
			if (!tree[index]->isParent ())
				continue;

			unsigned node = index / 2 + 1 + tree[index]->val;
			if (node > nodeEnd)
				--tree[index]->val;
		}
	}
}

void Node::encodeTree (std::vector<uint8_t> &tree, Node *node)
{
	std::vector<Node *> nodeTree (tree.size ());
	nodeTree[1] = node;
	serializeTree (nodeTree, node, 2);
	fixupTree (nodeTree);

#ifndef NDEBUG
	for (unsigned i = 1; i < nodeTree.size (); ++i)
	{
		assert (nodeTree[i]);
		nodeTree[i]->pos = i;
	}

	for (unsigned i = 1; i < nodeTree.size (); ++i)
	{
		node = nodeTree[i];
		if (!node->isParent ())
			continue;

		assert (!(node->val & 0x80));
		assert (!(node->val & 0x40));
		assert (node->child[0]->pos == (node->pos & ~1) + 2 * node->val + 2);
	}
#endif

	for (unsigned i = 1; i < nodeTree.size (); ++i)
	{
		node = nodeTree[i];

		tree[i] = node->val;

		if (!node->isParent ())
			continue;

		if (!node->child[0]->isParent ())
			tree[i] |= 0x80;
		if (!node->child[1]->isParent ())
			tree[i] |= 0x40;
	}
}

/** @brief Build Huffman tree
 *  @param[in] src Source data
 *  @param[in] len Source data length
 *  @param[in] fourBit_ Whether to use 4-bit encoding
 *  @returns Root node
 */
std::unique_ptr<Node> buildTree (const uint8_t *src, size_t len, bool fourBit_)
{
	// fill in histogram
	std::vector<size_t> histogram (fourBit_ ? 16 : 256);

	if (fourBit_)
	{
		for (size_t i = 0; i < len; ++i)
		{
			++histogram[(src[i] >> 0) & 0xF];
			++histogram[(src[i] >> 4) & 0xF];
		}
	}
	else
	{
		for (size_t i = 0; i < len; ++i)
			++histogram[src[i]];
	}

	std::vector<std::unique_ptr<Node>> nodes;
	{
		uint8_t val = 0;
		for (const auto &count : histogram)
		{
			if (count > 0)
				nodes.emplace_back (std::make_unique<Node> (val, count));

			++val;
		}
	}

	// done with histogram
	histogram.clear ();

	// combine nodes
	while (nodes.size () > 1)
	{
		// sort nodes by count; we will combine the two smallest nodes
		std::sort (std::begin (nodes),
		    std::end (nodes),
		    [] (const std::unique_ptr<Node> &lhs, const std::unique_ptr<Node> &rhs) -> bool {
			    return *lhs < *rhs;
		    });

		// allocate a parent node
		std::unique_ptr<Node> node =
		    std::make_unique<Node> (std::move (nodes[0]), std::move (nodes[1]));

		// replace first node with self
		nodes[0] = std::move (node);

		// replace second node with last node
		nodes[1] = std::move (nodes.back ());
		nodes.pop_back ();
	}

	// root is the last node left
	std::unique_ptr<Node> root = std::move (nodes[0]);

	// root must have children
	if (!root->isParent ())
		root = std::make_unique<Node> (std::move (root), std::make_unique<Node> (0x00, 0));

	// build Huffman codes
	Node::buildCodes (root, 0, 0);

	// return root node
	return root;
}

/** @brief Bitstream */
class Bitstream
{
public:
	Bitstream (std::vector<uint8_t> &buffer) : buffer (buffer)
	{
	}

	/** @brief Flush bitstream block, padded to 32 bits */
	void flush ()
	{
		if (pos >= 32)
			return;

		// append bitstream block to output buffer
		buffer.reserve (buffer.size () + 4);
		buffer.emplace_back (code >> 0);
		buffer.emplace_back (code >> 8);
		buffer.emplace_back (code >> 16);
		buffer.emplace_back (code >> 24);

		// reset bitstream block
		pos  = 32;
		code = 0;
	}

	/** @brief Push Huffman code onto bitstream
	 *  @param[in] code Huffman code
	 *  @param[in] len  Huffman code length (bits)
	 */
	void push (uint32_t code, size_t len)
	{
		for (size_t i = 1; i <= len; ++i)
		{
			// get next bit position
			--pos;

			// set/reset bit
			if (code & (1U << (len - i)))
				this->code |= (1U << pos);
			else
				this->code &= ~(1U << pos);

			// flush bitstream block
			if (pos == 0)
				flush ();
		}
	}

private:
	std::vector<uint8_t> &buffer; ///< Output buffer
	size_t pos    = 32;           ///< Bit position
	uint32_t code = 0;            ///< Bitstream block
};

std::vector<uint8_t> huffEncode (const void *source, size_t len, bool fourBit_)
{
	const uint8_t *src = (const uint8_t *)source;
	size_t count;

	// build Huffman tree
	std::unique_ptr<Node> root = buildTree (src, len, fourBit_);

	// build lookup table
	std::vector<Node *> lookup (256);
	Node::buildLookup (lookup, root);

	// get number of nodes
	count = root->numNodes ();

	// allocate Huffman encoded tree
	std::vector<uint8_t> tree ((count + 2) & ~1);

	// first slot encodes tree size
	tree[0] = count / 2;

	// encode Huffman tree
	Node::encodeTree (tree, root.get ());

	// create output buffer
	std::vector<uint8_t> result;
	result.reserve (len); // hopefully our output will be smaller

	// append compression header
	result.emplace_back (fourBit_ ? 0x24 : 0x28); // huff type
	result.emplace_back (len >> 0);
	result.emplace_back (len >> 8);
	result.emplace_back (len >> 16);

	if (len >= 0x1000000) // size extension, not compatible with BIOS routines!
	{
		result[0] |= 0x80;
		result.emplace_back (len >> 24);
		result.emplace_back (0);
		result.emplace_back (0);
		result.emplace_back (0);
	}

	// append Huffman encoded tree
	tree[0] = 0xFF;
	result.insert (std::end (result), std::begin (tree), std::end (tree));
	for (std::size_t i = tree.size (); i < 512; ++i)
		result.emplace_back (0x00);

	// we're done with the Huffman encoded tree
	tree.clear ();

	// create bitstream
	Bitstream bitstream (result);

	// encode each input byte
	if (fourBit_)
	{
		for (size_t i = 0; i < len; ++i)
		{
			// lookup the lower nibble's node
			Node *node = lookup[(src[i] >> 0) & 0xF];

			// add Huffman code to bitstream
			bitstream.push (node->getCode (), node->getCodeLen ());

			// lookup the upper nibble's node
			node = lookup[(src[i] >> 4) & 0xF];

			// add Huffman code to bitstream
			bitstream.push (node->getCode (), node->getCodeLen ());
		}
	}
	else
	{
		for (size_t i = 0; i < len; ++i)
		{
			// lookup the byte value's node
			Node *node = lookup[src[i]];

			// add Huffman code to bitstream
			bitstream.push (node->getCode (), node->getCodeLen ());
		}
	}

	// we're done with the Huffman tree and lookup table
	root.reset ();
	lookup.clear ();

	// flush the bitstream
	bitstream.flush ();

	// pad the output buffer to 4 bytes
	if (result.size () & 0x3)
		result.resize ((result.size () + 3) & ~0x3);

	// return the output data
	return result;
}

#ifndef NDEBUG
void huffDecode (const void *src, void *dst, size_t size, bool fourBit_)
{
	const uint8_t *in   = (const uint8_t *)src;
	uint8_t *out        = (uint8_t *)dst;
	uint32_t treeSize   = ((*in) + 1) * 2; // size of the huffman header
	uint32_t word       = 0;               // 32-bits of input bitstream
	uint32_t mask       = 0;               // which bit we are reading
	const uint8_t *tree = in;              // huffman tree
	size_t node;                           // node in the huffman tree
	size_t child;                          // child of a node
	uint32_t offset;                       // offset from node to child

	// point to the root of the huffman tree
	node = 1;

	// move input pointer to beginning of bitstream
	in += treeSize;

	uint8_t byte = 0;
	bool nibble = 0;

	auto pushData = [&]
	{
		// copy the child node into the output buffer
		if (fourBit_)
		{
			if (nibble)
			{
				*out++ = byte | (tree[child] << 4);
				--size;
			}
			else
				byte = tree[child];

			nibble = !nibble;
		}
		else
		{
			*out++ = tree[child];
			--size;
		}

		// start over at the root node
		node = 1;
	};

	while (size > 0)
	{
		if (mask == 0) // we exhausted 32 bits
		{
			// reset the mask
			mask = 0x80000000;

			// read the next 32 bits
			word = (in[0] << 0) | (in[1] << 8) | (in[2] << 16) | (in[3] << 24);
			in += 4;
		}

		// read the current node's offset value
		offset = tree[node] & 0x3F;

		child = (node & ~1) + offset * 2 + 2;

		if (word & mask) // we read a 1
		{
			// point to the "right" child
			++child;

			if (tree[node] & 0x40) // "right" child is a data node
				pushData ();
			else // traverse to the "right" child
				node = child;
		}
		else // we read a 0
		{
			// pointed to the "left" child

			if (tree[node] & 0x80) // "left" child is a data node
				pushData ();
			else // traverse to the "left" child
				node = child;
		}

		// shift to read next bit (read bit 31 to bit 0)
		mask >>= 1;
	}
}
#endif
}

uint huffgba_compress(RECORD *dst, const RECORD *src)
{
	if(!dst || !src || !src->data)
		return 0;

	auto const huff4 = huffEncode (src->data, rec_size (src), true);
	auto const huff8 = huffEncode (src->data, rec_size (src), false);

	auto const &huff = huff4.size () < huff8.size () ? huff4 : huff8;

	dst->width  = 1;
	dst->height = huff.size ();
	dst->data   = (BYTE*)malloc (huff.size ());
	memcpy (dst->data, huff.data (), huff.size ());

#ifndef NDEBUG
	std::vector<uint8_t> test (rec_size (src));

	assert (huff[0] == 0x24 || huff[0] == 0x28);
	huffDecode (&huff[4], test.data (), test.size (), huff[0] == 0x24);

	assert (huff[1] == ((rec_size (src) >> 0) & 0xFF));
	assert (huff[2] == ((rec_size (src) >> 8) & 0xFF));
	assert (huff[3] == ((rec_size (src) >> 16) & 0xFF));
	assert (memcmp (src->data, test.data (), test.size ()) == 0);
#endif

	return huff.size ();
}

================================================================================================================


# TODO
* Need some decisions:
  * Do we insist in solving huffman tree serialization ourselves, or do we simply want to get shit done?
    * If the latter, then using huffman compression from grit might be an option
    * Just check licensing again!
* Clean up tests to use new test directory thing
  * huffman_decoder_test:
    * Maybe place a readme somewhere that files have been created using reference encoders? (CUE Huffman, GBACrusher)
* We're past the point where testing against the public interface makes sense:
  * The tree serialization code does not need any actual input data. We need to be able to either
    * Feed it frequency tables
    * Or trees. Actually, the histogram produces a tree, which we then feed to the serializer, so we don't
      *need* frequency tables as input, but they're certainly simpler to construct than trees
  * So maybe we should find a way to do unit testing
    * If all else fails, compile the library once for unit testing and once for deployment
    * The library used for unit testing exports symbols the normal one does not export
    * How can we then ensure the library for unit testing
      * Does not get deployed/installed/whatever?
* OK: huffman tree serialization is a bitch
  * On the bright side: we're interested primarily in 4 bit coding for shrinkler-gba, so we could just call it a day :D
  * Our breadth-first traversal does not even work for a 256 byte file where each 8 bit symbol has the same frequency
  * Would a depth-first traversal work for this case? Maybe
  * Should we also write a maximum depth test?
  * Well, maybe. Prognosis:
    * breadth-first fails at even distribution, but does lucas sequence thing
    * depth-first fails at lucas sequence thing (orly? only if the short tree contains is more than one level deep?)
  * Well, we need to understand the problem.
    * A key to understanding is the following from grit's cprs_huff.cpp:
      ```
      if (node->numLeaves () > 0x40)
	  {
          // this subtree will overflow the offset field if inserted naively
          // ...
      }
      ```
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

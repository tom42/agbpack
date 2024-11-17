<!--
SPDX-FileCopyrightText: 2024 Thomas Mathys
SPDX-License-Identifier: MIT
-->

================================================================================================================
class Node
{
public:
	/** @brief Build lookup table
	 *  @param[in] nodes Table to fill
	 *  @param[in] n     Huffman node
	 */
	static void buildLookup (std::vector<Node *> &nodes, const std::unique_ptr<Node> &node);

	/** @brief Fixup serialized Huffman tree
	 *  @param[inout] tree Serialized tree
	 */
	static void fixupTree (std::vector<Node *> &tree);

	/** @brief Encode Huffman tree
	 *  @param[out] tree Huffman tree
	 *  @param[in]  node Huffman node
	 */
	static void encodeTree (std::vector<uint8_t> &tree, Node *node);

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
        assert (codeLen < 32);  ==>  Consider having this somewhere too, but as a runtime check, not assertion

        // Remaining uninteresting bits removed: this just built the huffman codes from the tree
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
}

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

        // ...
}

}

================================================================================================================


# TODO
* grit source code / issues claim that an 8 bit huffman tree needs to be 512 bytes.
  * Verify one last time this is NOT true => and maybe even document this?
* Put tree serialization code into own source file
  * Attribute where it's coming from
  * Also quote the statement that leads us to believe it's MIT licensed
* Need some decisions:
  * Do we insist in solving huffman tree serialization ourselves, or do we simply want to get shit done?
    * If the latter, then using huffman compression from grit might be an option
    * Just check licensing again!
	* Do not forget to update copyright notice in code
	  * Question is a bit, can we divide the huffman stuff a bit?
		* Would make things more overviewable
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

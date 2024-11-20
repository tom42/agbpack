// TODO: copyright is missing
// TODO: is this file using tabs ot what?

module;

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <memory>
#include <tuple>
#include <vector>

// TODO: temporarily disable warnings so that we've got a chance to compile anything at all
#if defined(_MSC_VER)
__pragma(warning(disable:4244))
__pragma(warning(disable:4267))
__pragma(warning(disable:4458))
#endif
#if defined(__clang__)
#pragma GCC diagnostic ignored "-Wimplicit-int-conversion"
#pragma GCC diagnostic ignored "-Wshadow"
#endif

module agbpack;

namespace agbpack_grit
{

namespace
{

using uint = unsigned int;

/** @brief Huffman node */
class Node
{
public:
	/** @brief Parameterized constructor
	 *  @param val   Node value
     *  @param count Node count
	 */
	Node(uint8_t val, size_t count) : count(count), val(val)
	{
	}

	/** @brief Parameterized constructor
	 *  @param left  Left child
     *  @param right Right child
	 */
	Node(std::unique_ptr<Node> left, std::unique_ptr<Node> right)
		: child{ std::move(left), std::move(right) }, count(child[0]->count + child[1]->count)
	{
	}

	Node() = delete;

	Node(const Node& other) = delete;

	Node(Node&& other) = delete;

	Node& operator= (const Node& other) = delete;

	Node& operator= (Node&& other) = delete;

	/** @brief Comparison operator
	 *  @param other Object to compare
	 */
	bool operator< (const Node& other) const
	{
		// major key is count
		if (count != other.count)
			return count < other.count;

		// minor key is value
		return val < other.val;
	}

	/** @brief Whether this node is a parent */
	bool isParent() const
	{
		return static_cast<bool> (child[0]);
	}

	/** @brief Build Huffman codes
	 *  @param[in] node    Huffman node
	 *  @param[in] code    Huffman code
	 *  @param[in] codeLen Huffman code length (bits)
	 */
	static void buildCodes(std::unique_ptr<Node>& node, uint32_t code, size_t codeLen);

	/** @brief Build lookup table
	 *  @param[in] nodes Table to fill
     *  @param[in] node  Huffman node
	 */
	static void buildLookup(std::vector<Node*>& nodes, const std::unique_ptr<Node>& node);

	/** @brief Serialize Huffman tree
	 *  @param[out] tree Serialized tree
	 *  @param[in]  node Root of subtree
	 *  @param[in]  next Next available slot in tree
	 */
	static void serializeTree(std::vector<Node*>& tree, Node* node, unsigned next);

	/** @brief Fixup serialized Huffman tree
     *  @param[in,out] tree Serialized tree
	 */
	static void fixupTree(std::vector<Node*>& tree);

	/** @brief Encode Huffman tree
	 *  @param[out] tree Huffman tree
	 *  @param[in]  node Huffman node
	 */
	static void encodeTree(std::vector<uint8_t>& tree, Node* node);

	/** @brief Count number of nodes in subtree
	 *  @returns Number of nodes in subtree
	 */
	size_t numNodes() const
	{
		// sum of children plus self
		if (isParent())
			return child[0]->numNodes() + child[1]->numNodes() + 1;

		// this is a data node, just count self
		return 1;
	}

	/** @brief Count number of leaves in subtree
	 *  @returns Number of leaves in subtree
	 */
	size_t numLeaves()
	{
		if (leaves == 0)
		{
			if (isParent())
			{
				// sum of children
				leaves = child[0]->numLeaves() + child[1]->numLeaves();
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
	uint32_t getCode() const
	{
		assert(!isParent());
		return code;
	}

	/** @brief Get code length */
	uint8_t getCodeLen() const
	{
		assert(!isParent());
		return codeLen;
	}

private:
    std::array<std::unique_ptr<Node>, 2> child;
    size_t count = 0;
    uint32_t code = 0;
    unsigned leaves = 0;
    uint8_t val = 0;
    uint8_t codeLen = 0;
#ifndef NDEBUG // TODO: do we not want this sanity check always?
    uint16_t pos = 0;
#endif
};

void Node::buildCodes(std::unique_ptr<Node>& node, uint32_t code, size_t codeLen)
{
	// don't exceed 32-bit codes
	assert(codeLen < 32);

	if (node->isParent())
	{
		// build codes for each subtree
		assert(node->child[0] && node->child[1]);
		buildCodes(node->child[0], (code << 1) | 0, codeLen + 1);
		buildCodes(node->child[1], (code << 1) | 1, codeLen + 1);
	}
	else
	{
		// set code for data node
		assert(!node->child[0] && !node->child[1]);
		node->code = code;
		node->codeLen = codeLen;
	}
}

void Node::buildLookup(std::vector<Node*>& nodes, const std::unique_ptr<Node>& node)
{
	if (!node->isParent())
	{
		// set lookup entry
		nodes[node->val] = node.get();
		return;
	}

	// build subtree lookups
	buildLookup(nodes, node->child[0]);
	buildLookup(nodes, node->child[1]);
}

void Node::serializeTree(std::vector<Node*>& tree, Node* node, unsigned next)
{
	assert(node->isParent());

	if (node->numLeaves() > 0x40)
	{
		// this subtree will overflow the offset field if inserted naively
		tree[next + 0] = node->child[0].get();
		tree[next + 1] = node->child[1].get();

		unsigned a = 0;
		unsigned b = 1;

		if (node->child[1]->numLeaves() < node->child[0]->numLeaves())
			std::swap(a, b);

		if (node->child[a]->isParent())
		{
			node->child[a]->val = 0;
			serializeTree(tree, node->child[a].get(), next + 2);
		}

		if (node->child[b]->isParent())
		{
			node->child[b]->val = node->child[a]->numLeaves() - 1;
			serializeTree(tree, node->child[b].get(), next + 2 * node->child[a]->numLeaves());
		}

		return;
	}

	std::deque<Node*> queue;

	queue.emplace_back(node->child[0].get());
	queue.emplace_back(node->child[1].get());

	while (!queue.empty())
	{
		node = queue.front();
		queue.pop_front();

		tree[next++] = node;

		if (!node->isParent())
			continue;

		node->val = queue.size() / 2;

		queue.emplace_back(node->child[0].get());
		queue.emplace_back(node->child[1].get());
	}
}

void Node::fixupTree(std::vector<Node*>& tree)
{
	for (unsigned i = 1; i < tree.size(); ++i)
	{
		if (!tree[i]->isParent() || tree[i]->val <= 0x3F)
			continue;

		unsigned shift = tree[i]->val - 0x3F;

		if ((i & 1) && tree[i - 1]->val == 0x3F)
		{
			// right child, and left sibling would overflow if we shifted;
			// shift the left child by 1 instead
			--i;
			shift = 1;
		}

		unsigned nodeEnd = i / 2 + 1 + tree[i]->val;
		unsigned nodeBegin = nodeEnd - shift;

		unsigned shiftBegin = 2 * nodeBegin;
		unsigned shiftEnd = 2 * nodeEnd;

		// move last child pair to front
		auto tmp = std::make_pair(tree[shiftEnd], tree[shiftEnd + 1]);
		std::memmove(
			&tree[shiftBegin + 2], &tree[shiftBegin], sizeof(Node*) * (shiftEnd - shiftBegin));
		std::tie(tree[shiftBegin], tree[shiftBegin + 1]) = tmp;

		// adjust offsets
		tree[i]->val -= shift;
		for (unsigned index = i + 1; index < shiftBegin; ++index)
		{
			if (!tree[index]->isParent())
				continue;

			unsigned node = index / 2 + 1 + tree[index]->val;
			if (node >= nodeBegin && node < nodeEnd)
				++tree[index]->val;
		}

		if (tree[shiftBegin + 0]->isParent())
			tree[shiftBegin + 0]->val += shift;
		if (tree[shiftBegin + 1]->isParent())
			tree[shiftBegin + 1]->val += shift;

		for (unsigned index = shiftBegin + 2; index < shiftEnd + 2; ++index)
		{
			if (!tree[index]->isParent())
				continue;

			unsigned node = index / 2 + 1 + tree[index]->val;
			if (node > nodeEnd)
				--tree[index]->val;
		}
	}
}

void Node::encodeTree(std::vector<uint8_t>& tree, Node* node)
{
	std::vector<Node*> nodeTree(tree.size());
	nodeTree[1] = node;
	serializeTree(nodeTree, node, 2);
	fixupTree(nodeTree);

#ifndef NDEBUG
	for (unsigned i = 1; i < nodeTree.size(); ++i)
	{
		assert(nodeTree[i]);
		nodeTree[i]->pos = i;
	}

	for (unsigned i = 1; i < nodeTree.size(); ++i)
	{
		node = nodeTree[i];
		if (!node->isParent())
			continue;

		assert(!(node->val & 0x80));
		assert(!(node->val & 0x40));
		assert(node->child[0]->pos == (node->pos & ~1) + 2 * node->val + 2);
	}
#endif

	for (unsigned i = 1; i < nodeTree.size(); ++i)
	{
		node = nodeTree[i];

		tree[i] = node->val;

		if (!node->isParent())
			continue;

		if (!node->child[0]->isParent())
			tree[i] |= 0x80;
		if (!node->child[1]->isParent())
			tree[i] |= 0x40;
	}
}

/** @brief Build Huffman tree
 *  @param[in] src Source data
 *  @param[in] len Source data length
 *  @param[in] fourBit_ Whether to use 4-bit encoding
 *  @returns Root node
 */
std::unique_ptr<Node> buildTree(const uint8_t* src, size_t len, bool fourBit_)
{
	// fill in histogram
	std::vector<size_t> histogram(fourBit_ ? 16 : 256);

    // Temporarily suppress this warning;
    // We plan to use our own histogram generation code, so we're not going to fix this anyway
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunsafe-buffer-usage"
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
        {
            ++histogram[src[i]];
        }
	}
#pragma GCC diagnostic pop

	std::vector<std::unique_ptr<Node>> nodes;
	{
		uint8_t val = 0;
		for (const auto& count : histogram)
		{
			if (count > 0)
				nodes.emplace_back(std::make_unique<Node>(val, count));

			++val;
		}
	}

	// done with histogram
	histogram.clear();

	// Fix: if there are less than 2 nodes, add bogus nodes
	while (nodes.size() < 2)
	{
		nodes.emplace_back(std::make_unique<Node>(0, 0));
	}

	// combine nodes
	while (nodes.size() > 1)
	{
		// sort nodes by count; we will combine the two smallest nodes
		std::sort(std::begin(nodes),
			std::end(nodes),
			[](const std::unique_ptr<Node>& lhs, const std::unique_ptr<Node>& rhs) -> bool {
				return *lhs < *rhs;
			});

		// allocate a parent node
		std::unique_ptr<Node> node =
			std::make_unique<Node>(std::move(nodes[0]), std::move(nodes[1]));

		// replace first node with self
		nodes[0] = std::move(node);

		// replace second node with last node
		nodes[1] = std::move(nodes.back());
		nodes.pop_back();
	}

	// root is the last node left
	std::unique_ptr<Node> root = std::move(nodes[0]);

	// root must have children
	if (!root->isParent())
		root = std::make_unique<Node>(std::move(root), std::make_unique<Node>(0x00, 0));

	// build Huffman codes
	Node::buildCodes(root, 0, 0);

	// return root node
	return root;
}

/** @brief Bitstream */
class Bitstream
{
public:
	Bitstream(std::vector<uint8_t>& buffer) : buffer(buffer)
	{
	}

	/** @brief Flush bitstream block, padded to 32 bits */
	void flush()
	{
		if (pos >= 32)
			return;

		// append bitstream block to output buffer
		buffer.reserve(buffer.size() + 4);
		buffer.emplace_back(code >> 0);
		buffer.emplace_back(code >> 8);
		buffer.emplace_back(code >> 16);
		buffer.emplace_back(code >> 24);

		// reset bitstream block
		pos = 32;
		code = 0;
	}

	/** @brief Push Huffman code onto bitstream
	 *  @param[in] code Huffman code
	 *  @param[in] len  Huffman code length (bits)
	 */
	void push(uint32_t code, size_t len)
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
				flush();
		}
	}

private:
	std::vector<uint8_t>& buffer; ///< Output buffer
	size_t pos = 32;           ///< Bit position
	uint32_t code = 0;            ///< Bitstream block
};

}

std::vector<uint8_t> huffEncode(const void* source, size_t len, bool fourBit_)
{
    auto src = static_cast<const uint8_t*>(source); // TODO: no cast here
	size_t count;

	// build Huffman tree
	std::unique_ptr<Node> root = buildTree(src, len, fourBit_);

	// build lookup table
	std::vector<Node*> lookup(256);
	Node::buildLookup(lookup, root);

	// get number of nodes
	count = root->numNodes();

	// allocate Huffman encoded tree
    // Original version commented out since it generates warnings.
    // Anyway it should be much simpler:
    // The number of nodes in a huffman tree should always be odd,
    // so we can just add +1 for the tree size byte and move on
    std::vector<uint8_t> tree(count + 1);

	// first slot encodes tree size
	tree[0] = count / 2;

	// encode Huffman tree
	Node::encodeTree(tree, root.get());

	// create output buffer
	std::vector<uint8_t> result;
	result.reserve(len); // hopefully our output will be smaller

	// append compression header
	result.emplace_back(fourBit_ ? 0x24 : 0x28); // huff type
	result.emplace_back(len >> 0);
	result.emplace_back(len >> 8);
	result.emplace_back(len >> 16);

	if (len >= 0x1000000) // size extension, not compatible with BIOS routines!
	{
		result[0] |= 0x80;
		result.emplace_back(len >> 24);
		result.emplace_back(0);
		result.emplace_back(0);
		result.emplace_back(0);
	}

	// append Huffman encoded tree
	// Note: original version is commented out because it's broken.
	/*tree[0] = 0xFF;
	result.insert(std::end(result), std::begin(tree), std::end(tree));
	for (std::size_t i = tree.size(); i < 512; ++i)
		result.emplace_back(0x00);*/

	while (tree.size() % 4 != 0) { tree.push_back(0); }                 // Make tree size a multiple of 4 bytes
	tree[0] = tree.size() / 2 - 1;                                      // Write correct tree size byte
	result.insert(std::end(result), std::begin(tree), std::end(tree));  // Append tree to output

	// we're done with the Huffman encoded tree
	tree.clear();

	// create bitstream
	Bitstream bitstream(result);

	// encode each input byte
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunsafe-buffer-usage"
	if (fourBit_)
	{
		for (size_t i = 0; i < len; ++i)
		{
			// lookup the lower nibble's node
			Node* node = lookup[(src[i] >> 0) & 0xF];

			// add Huffman code to bitstream
			bitstream.push(node->getCode(), node->getCodeLen());

			// lookup the upper nibble's node
			node = lookup[(src[i] >> 4) & 0xF];

			// add Huffman code to bitstream
			bitstream.push(node->getCode(), node->getCodeLen());
		}
	}
	else
	{
		for (size_t i = 0; i < len; ++i)
		{
			// lookup the byte value's node
			Node* node = lookup[src[i]];

			// add Huffman code to bitstream
			bitstream.push(node->getCode(), node->getCodeLen());
		}
	}
#pragma GCC diagnostic pop

	// flush the bitstream
	bitstream.flush();

	// pad the output buffer to 4 bytes
    // Note: commented out: causes warnings and should not be necessary since flush()
    // should have left things already aligned
    // if (result.size() & 0x3)
    // 	result.resize((result.size() + 3) & ~0x3);

	// return the output data
	return result;
}

}

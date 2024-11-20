// TODO: copyright is missing
// TODO: is this file using tabs or what?

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

module agbpack;

namespace agbpack_grit
{

namespace
{

class Node
{
public:
    Node(uint8_t val, size_t count)
        : m_count(count), m_val(val)
    {}

    Node(std::unique_ptr<Node> left, std::unique_ptr<Node> right)
        : child{ std::move(left), std::move(right) }, m_count(child[0]->m_count + child[1]->m_count)
    {}

    Node() = delete;

    Node(const Node& other) = delete;

    Node(Node&& other) = delete;

    Node& operator=(const Node& other) = delete;

    Node& operator=(Node&& other) = delete;

    bool operator<(const Node& other) const
    {
        if (m_count != other.m_count)
        {
            return m_count < other.m_count;
        }

        return m_val < other.m_val;
	}

    bool isParent() const
    {
        return static_cast<bool> (child[0]);
    }

    static void buildCodes(std::unique_ptr<Node>& node, uint32_t code, size_t codeLen);

    static void buildLookup(std::vector<Node*>& nodes, const std::unique_ptr<Node>& node);

    static void serializeTree(std::vector<Node*>& tree, Node* node, std::size_t next);

    static void fixupTree(std::vector<Node*>& tree);

    static void encodeTree(std::vector<uint8_t>& tree, Node* node);

    // Returns the number of nodes in this subtree
    size_t numNodes() const
    {
        if (isParent())
        {
            // Sum of children plus self
            return child[0]->numNodes() + child[1]->numNodes() + 1;
        }

        // This is a data node, just count self
        return 1;
    }

    // Returns the number of leaves in this subtree
    size_t numLeaves()
    {
        if (leaves == 0)
        {
            if (isParent())
            {
                leaves = child[0]->numLeaves() + child[1]->numLeaves();
            }
            else
            {
                leaves = 1;
            }
        }

        return leaves;
    }

    uint32_t getCode() const
    {
        assert(!isParent());
        return code;
    }

    std::size_t getCodeLen() const
    {
        assert(!isParent());
        return codeLen;
    }

private:
    std::array<std::unique_ptr<Node>, 2> child{};
    size_t m_count = 0;
    uint32_t code = 0;
    std::size_t leaves = 0;
    uint8_t m_val = 0;
    std::size_t codeLen = 0;
#ifndef NDEBUG // TODO: do we not want this sanity check always?
    std::size_t pos = 0;
#endif
};

void Node::buildCodes(std::unique_ptr<Node>& node, uint32_t code, size_t codeLen)
{
    // TODO: this should be a runtime check, no?
    // don't exceed 32-bit codes
    assert(codeLen < 32);

    if (node->isParent())
    {
        assert(node->child[0] && node->child[1]);
        buildCodes(node->child[0], (code << 1) | 0, codeLen + 1);
        buildCodes(node->child[1], (code << 1) | 1, codeLen + 1);
    }
    else
    {
        assert(!node->child[0] && !node->child[1]);
        node->code = code;
        node->codeLen = codeLen;
    }
}

void Node::buildLookup(std::vector<Node*>& nodes, const std::unique_ptr<Node>& node)
{
    if (!node->isParent())
    {
        nodes[node->m_val] = node.get();
        return;
    }

    buildLookup(nodes, node->child[0]);
    buildLookup(nodes, node->child[1]);
}

void Node::serializeTree(std::vector<Node*>& tree, Node* node, std::size_t next)
{
    assert(node->isParent());

    if (node->numLeaves() > 0x40)
    {
        // This subtree will overflow the offset field if inserted naively
        tree[next + 0] = node->child[0].get();
        tree[next + 1] = node->child[1].get();

        unsigned a = 0;
        unsigned b = 1;

        if (node->child[1]->numLeaves() < node->child[0]->numLeaves())
        {
            std::swap(a, b);
        }

        if (node->child[a]->isParent())
        {
            node->child[a]->m_val = 0;
            serializeTree(tree, node->child[a].get(), next + 2);
        }

        if (node->child[b]->isParent())
        {
            // TODO: no cast?
            node->child[b]->m_val = static_cast<uint8_t>(node->child[a]->numLeaves() - 1);
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
        {
            continue;
        }

        // TODO: no cast
        node->m_val = static_cast<uint8_t>(queue.size() / 2);

        queue.emplace_back(node->child[0].get());
        queue.emplace_back(node->child[1].get());
    }
}

void Node::fixupTree(std::vector<Node*>& tree)
{
    for (unsigned i = 1; i < tree.size(); ++i)
    {
        if (!tree[i]->isParent() || tree[i]->m_val <= 0x3F)
        {
            continue;
        }

        unsigned shift = tree[i]->m_val - 0x3F;

        if ((i & 1) && tree[i - 1]->m_val == 0x3F)
        {
            // Right child, and left sibling would overflow if we shifted;
            // Shift the left child by 1 instead
            --i;
            shift = 1;
        }

        unsigned nodeEnd = i / 2 + 1 + tree[i]->m_val;
        unsigned nodeBegin = nodeEnd - shift;

        unsigned shiftBegin = 2 * nodeBegin;
        unsigned shiftEnd = 2 * nodeEnd;

        // Move last child pair to front
        auto tmp = std::make_pair(tree[shiftEnd], tree[shiftEnd + 1]);
        std::memmove(&tree[shiftBegin + 2], &tree[shiftBegin], sizeof(Node*) * (shiftEnd - shiftBegin));
        std::tie(tree[shiftBegin], tree[shiftBegin + 1]) = tmp;

        // Adjust offsets
        tree[i]->m_val -= shift;
        for (unsigned index = i + 1; index < shiftBegin; ++index)
        {
            if (!tree[index]->isParent())
            {
                continue;
            }

            unsigned node = index / 2 + 1 + tree[index]->m_val;
            if (node >= nodeBegin && node < nodeEnd)
            {
                ++tree[index]->m_val;
            }
        }

        if (tree[shiftBegin + 0]->isParent())
        {
            tree[shiftBegin + 0]->m_val += shift;
        }
        if (tree[shiftBegin + 1]->isParent())
        {
            tree[shiftBegin + 1]->m_val += shift;
        }

        for (unsigned index = shiftBegin + 2; index < shiftEnd + 2; ++index)
        {
            if (!tree[index]->isParent())
            {
                continue;
            }

            unsigned node = index / 2 + 1 + tree[index]->m_val;
            if (node > nodeEnd)
            {
                --tree[index]->m_val;
            }
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
        {
            continue;
        }

        assert(!(node->m_val & 0x80));
        assert(!(node->m_val & 0x40));
        assert(node->child[0]->pos == (node->pos & ~1u) + 2 * node->m_val + 2);
    }
#endif

    for (unsigned i = 1; i < nodeTree.size(); ++i)
    {
        node = nodeTree[i];

        tree[i] = node->m_val;

        if (!node->isParent())
        {
            continue;
        }

        if (!node->child[0]->isParent())
        {
            tree[i] |= 0x80;
        }
        if (!node->child[1]->isParent())
        {
            tree[i] |= 0x40;
        }
    }
}

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
            {
                nodes.emplace_back(std::make_unique<Node>(val, count));
            }

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
        std::unique_ptr<Node> node = std::make_unique<Node>(std::move(nodes[0]), std::move(nodes[1]));

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
    {
        root = std::make_unique<Node>(std::move(root), std::make_unique<Node>(0x00, 0));
    }

    // build Huffman codes
    Node::buildCodes(root, 0, 0);

    // return root node
    return root;
}

class Bitstream
{
public:
    Bitstream(std::vector<uint8_t>& buffer) : m_buffer(buffer) {}

    void flush()
    {
        if (pos >= 32)
            return;

        // append bitstream block to output buffer
        m_buffer.reserve(m_buffer.size() + 4);
        m_buffer.emplace_back(m_code >> 0);
        m_buffer.emplace_back(m_code >> 8);
        m_buffer.emplace_back(m_code >> 16);
        m_buffer.emplace_back(m_code >> 24);

        // reset bitstream block
        pos = 32;
        m_code = 0;
    }

    void push(uint32_t code, size_t len)
    {
        for (size_t i = 1; i <= len; ++i)
        {
            // get next bit position
            --pos;

            // set/reset bit
            if (code & (1U << (len - i)))
                this->m_code |= (1U << pos);
            else
                this->m_code &= ~(1U << pos);

            // flush bitstream block
            if (pos == 0)
                flush();
        }
    }

private:
    std::vector<uint8_t>& m_buffer;
    size_t pos = 32;
    uint32_t m_code = 0;
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
    // Commented out: generates warning, and below we recalculate it anyway
    //tree[0] = count / 2;

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
    // TODO: no cast?
    while (tree.size() % 4 != 0) { tree.push_back(0); }                 // Make tree size a multiple of 4 bytes
    tree[0] = static_cast<uint8_t>(tree.size() / 2 - 1);                // Write correct tree size byte
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

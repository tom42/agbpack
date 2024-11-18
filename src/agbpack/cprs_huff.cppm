// TODO: copyright is missing
// TODO: delete

module;

#include <cstddef>
#include <vector>

export module agbpack:cprs_huff;

namespace agbpack
{

// TODO: should really fully qualify size_t here
std::vector<uint8_t> huffEncode(const void* source, size_t len, bool fourBit_);

}
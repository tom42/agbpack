// TODO: copyright is missing
// TODO: delete

module;

#include <cstddef>
#include <cstdint>
#include <vector>

export module agbpack:cprs_huff;

namespace agbpack
{

// TODO: should really fully qualify size_t here
export std::vector<uint8_t> huffEncode(const void* source, size_t len, bool fourBit_);

}

// SPDX-FileCopyrightText: 2026 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>

import agbpack;

int main()
{
    try
    {
        const std::vector<unsigned char> uncompressed(32, 'a');
        std::vector<unsigned char> compressed;
        agbpack::lzss_encoder e;

        e.encode(begin(uncompressed), end(uncompressed), back_inserter(compressed));

        std::cout << "Uncompressed size: " << uncompressed.size() << " bytes\n";
        std::cout << "Compressed size:   " << compressed.size() << " bytes\n";

        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
}

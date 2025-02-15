# SPDX-FileCopyrightText: 2025 Thomas Mathys
# SPDX-License-Identifier: MIT

add_executable(lzss-analyze lzss-analyze.cpp)
target_link_libraries(lzss-analyze PRIVATE agbpack)

add_executable(lzss-encode lzss-encode.cpp)
target_link_libraries(lzss-encode PRIVATE agbpack)

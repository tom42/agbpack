# SPDX-FileCopyrightText: 2025 Thomas Mathys
# SPDX-License-Identifier: MIT

add_executable(lzss-analyze lzss-analyze.cpp)
target_link_libraries(lzss-analyze PRIVATE agbpack)

add_executable(lzss-encode lzss-encode.cpp)
target_link_libraries(lzss-encode PRIVATE agbpack)

# TODO: we don't want to install these by default, no? This is just debug stuff for our own purpose...
install(TARGETS lzss-analyze lzss-encode DESTINATION bin)

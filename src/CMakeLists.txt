# SPDX-FileCopyrightText: 2024 Thomas Mathys
# SPDX-License-Identifier: MIT

add_subdirectory(agbpack)

if(agbpack_BUILD_AGBPACKER)
  add_subdirectory(agbpacker)
endif()

if(agbpack_BUILD_LZSS_ANALYZE)
  add_subdirectory(lzss-analyze)
endif()

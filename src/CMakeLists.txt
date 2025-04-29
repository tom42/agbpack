# SPDX-FileCopyrightText: 2024 Thomas Mathys
# SPDX-License-Identifier: MIT

add_subdirectory(agbpack)

if(agbpack_BUILD_AGBPACKER)
  add_subdirectory(agbpacker)
endif()

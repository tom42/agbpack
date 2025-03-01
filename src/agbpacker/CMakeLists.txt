# SPDX-FileCopyrightText: 2025 Thomas Mathys
# SPDX-License-Identifier: MIT

add_executable(agbpacker agbpacker.cpp)
target_link_libraries(agbpacker PRIVATE agbpack argpppp)
install(TARGETS agbpacker DESTINATION bin)

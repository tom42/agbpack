# SPDX-FileCopyrightText: 2024 Thomas Mathys
# SPDX-License-Identifier: MIT

add_executable(agbpack_test add_test.cpp)
target_link_libraries(agbpack_test PRIVATE agbpack Catch2::Catch2WithMain)

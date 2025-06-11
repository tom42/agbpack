# SPDX-FileCopyrightText: 2025 Thomas Mathys
# SPDX-License-Identifier: MIT

include(FetchContent)
FetchContent_Declare(
  argpppp
  GIT_REPOSITORY https://github.com/tom42/argpppp
  GIT_TAG        v0.0.8)
FetchContent_MakeAvailable(argpppp)

add_executable(agbpacker agbpacker.cpp)
vtg_target_enable_warnings(agbpacker)
target_link_libraries(agbpacker PRIVATE agbpack argpppp)
install(TARGETS agbpacker DESTINATION bin)

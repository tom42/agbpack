# SPDX-FileCopyrightText: 2025 Thomas Mathys
# SPDX-License-Identifier: MIT

# TODO: this might have to move to agbpacker_core
include(FetchContent)
FetchContent_Declare(
  argpppp
  GIT_REPOSITORY https://github.com/tom42/argpppp
  GIT_TAG        v0.4.0)
FetchContent_MakeAvailable(argpppp)

add_executable(agbpacker agbpacker.cpp)
vtg_target_enable_warnings(agbpacker)
target_link_libraries(agbpacker PRIVATE agbpacker_core)

if(agbpack_INSTALL_AGBPACKER)
  include(GNUInstallDirs)
  install(TARGETS agbpacker DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

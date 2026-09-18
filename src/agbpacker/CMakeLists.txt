# SPDX-FileCopyrightText: 2025 Thomas Mathys
# SPDX-License-Identifier: MIT

add_executable(agbpacker agbpacker.cpp)
vtg_target_enable_warnings(agbpacker)
target_link_libraries(agbpacker PRIVATE agbpacker_core)

if(agbpack_INSTALL_AGBPACKER)
  include(GNUInstallDirs)
  install(TARGETS agbpacker DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

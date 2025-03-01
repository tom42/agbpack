# SPDX-FileCopyrightText: 2025 Thomas Mathys
# SPDX-License-Identifier: MIT

set(
  argpppp_sources
  PUBLIC FILE_SET CXX_MODULES FILES
  argpppp.cppm
  PRIVATE
  argpppp.cpp)

add_library(argpppp)
target_sources(argpppp ${argpppp_sources})

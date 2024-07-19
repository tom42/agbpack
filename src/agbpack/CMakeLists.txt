# SPDX-FileCopyrightText: 2024 Thomas Mathys
# SPDX-License-Identifier: MIT

add_library(agbpack)
target_sources(
  agbpack
  PUBLIC FILE_SET CXX_MODULES FILES
  agbpack.cppm
  agbpack-common.cppm
  agbpack-delta.cppm
  agbpack-exceptions.cppm
  agbpack-lzss.cppm
  agbpack-header.cppm
  agbpack-rle.cppm)

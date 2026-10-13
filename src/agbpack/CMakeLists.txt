# SPDX-FileCopyrightText: 2024 Thomas Mathys
# SPDX-License-Identifier: MIT

# clownlzss does not build with our warning flags.
# We compile it as a separate object library without warnings enabled and then
# include it in agbpack. Not having warnings enabled is acceptable here, since
# this is the unmodified source code from the original repository.
# TODO: random cleanup
#       * Document what we're doing here
#         * Why are we not using target_include_directories?
#         * Why do we need to install the library (See https://stackoverflow.com/questions/40183810/why-does-cmake-export-require-static-libraries-used-to-build-a-shared-library and CMake doc linked by that page)
add_library(clownlzss_objects OBJECT clownlzss/clownlzss.c)

set(
  agbpack_sources
  PUBLIC FILE_SET CXX_MODULES FILES
  agbpack.cppm
  common.cppm
  delta.cppm
  exceptions.cppm
  huffman.cppm
  lzss.cppm
  header.cppm
  rle.cppm
  PRIVATE
  lzss.cpp
  header.cpp)

# Production version of agbpack
add_library(agbpack)
target_sources(agbpack ${agbpack_sources})
target_compile_definitions(agbpack PRIVATE AGBPACK_EXPORT_FOR_UNIT_TESTING=)
target_compile_features(agbpack PUBLIC cxx_std_23)
target_link_libraries(agbpack PRIVATE clownlzss_objects)
vtg_target_enable_warnings(agbpack)

# Unit testing version of agbpack
if(agbpack_BUILD_TESTING)
  add_library(agbpack_unit_testing)
  target_sources(agbpack_unit_testing ${agbpack_sources})
  target_compile_definitions(agbpack_unit_testing PRIVATE AGBPACK_EXPORT_FOR_UNIT_TESTING=export)
  target_compile_features(agbpack_unit_testing PUBLIC cxx_std_23)
  target_link_libraries(agbpack_unit_testing PRIVATE clownlzss_objects)
  vtg_target_enable_warnings(agbpack_unit_testing)
endif()

if(agbpack_INSTALL_STATIC_LIBRARY)
  include(InstallAgbpack)
endif()

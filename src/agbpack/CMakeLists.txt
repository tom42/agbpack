# SPDX-FileCopyrightText: 2024 Thomas Mathys
# SPDX-License-Identifier: MIT

# clownlzss does not build with our warning flags.
# We compile it as a separate object library without warnings enabled and then
# include it in agbpack. Not having warnings enabled is acceptable here, since
# this is the unmodified source code from the original repository.
# TODO: the way we're using clownlzss_objects now (with target_link_libraries) breaks installation
#       * Either revert it (use generator expression to pull in objects and target_include_directories)
#       * Or attempt to fix it (https://stackoverflow.com/questions/40183810/why-does-cmake-export-require-static-libraries-used-to-build-a-shared-library)
#       * Or compile it as part of agbpack_sources, problem here is the warnings
add_library(clownzss_objects OBJECT clownlzss/clownlzss.c)
target_include_directories(clownzss_objects PUBLIC clownlzss)

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
target_link_libraries(agbpack PRIVATE clownzss_objects)
vtg_target_enable_warnings(agbpack)

# Unit testing version of agbpack
if(agbpack_BUILD_TESTING)
  add_library(agbpack_unit_testing)
  target_sources(agbpack_unit_testing ${agbpack_sources})
  target_compile_definitions(agbpack_unit_testing PRIVATE AGBPACK_EXPORT_FOR_UNIT_TESTING=export)
  target_compile_features(agbpack_unit_testing PUBLIC cxx_std_23)
  target_link_libraries(agbpack_unit_testing PRIVATE clownzss_objects)
  vtg_target_enable_warnings(agbpack_unit_testing)
endif()

if(agbpack_INSTALL_STATIC_LIBRARY)
  include(InstallAgbpack)
endif()

# SPDX-FileCopyrightText: 2024 Thomas Mathys
# SPDX-License-Identifier: MIT

# clownlzss does not build with our warning flags.
# We compile it as a separate object library without warnings enabled and then
# include it in agbpack. Not having warnings enabled is acceptable here, since
# this is the unmodified source code from the original repository.
add_library(clownzss_objects OBJECT clownlzss/clownlzss.c)

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
  header.cpp)

 # Make MSVC correctly define __cplusplus, so that we can use all the C++ specific things provided by the clownlzss.h header.
 # See also:
 # * https://gitlab.kitware.com/cmake/cmake/-/issues/18837
 # * https://learn.microsoft.com/en-us/cpp/build/reference/zc-cplusplus
if ((MSVC) AND (MSVC_VERSION GREATER_EQUAL 1914))
  set(agbpack_compile_options /Zc:__cplusplus)
endif()

# Production version of agbpack.
add_library(agbpack $<TARGET_OBJECTS:clownzss_objects>)
target_sources(agbpack ${agbpack_sources})
target_compile_definitions(agbpack PRIVATE AGBPACK_EXPORT_FOR_UNIT_TESTING=)
target_compile_features(agbpack PUBLIC cxx_std_23)
target_compile_options(agbpack PRIVATE ${agbpack_compile_options})
target_include_directories(agbpack PRIVATE clownlzss)
vtg_target_enable_warnings(agbpack)

# Unit testing version of agbpack.
if(agbpack_BUILD_TESTING)
  add_library(agbpack_unit_testing $<TARGET_OBJECTS:clownzss_objects>)
  target_sources(agbpack_unit_testing ${agbpack_sources})
  target_compile_definitions(agbpack_unit_testing PRIVATE AGBPACK_EXPORT_FOR_UNIT_TESTING=export)
  target_compile_features(agbpack_unit_testing PUBLIC cxx_std_23)
  target_compile_options(agbpack_unit_testing PRIVATE ${agbpack_compile_options})
  target_include_directories(agbpack_unit_testing PRIVATE clownlzss)
  vtg_target_enable_warnings(agbpack_unit_testing)
endif()

if(agbpack_INSTALL_STATIC_LIBRARY)
  include(InstallAgbpack)
endif()

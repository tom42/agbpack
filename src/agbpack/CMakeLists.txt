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
#       * Do something useful inside agbpack_install_test.cpp (like actually referencing some symbol from agbpack, and printing something, like some header. Or maybe compressing something)
#         * Possibly run the binary as a test (that is, do something like add_test(agbpack_install_test ...) (details to be worked out, you get the idea)
#       * The installation test does break. If we install into /tmp, then, if we try to use agbpack installed there, find_package works, but when we try to use the library
#         using target_link_libraries we get a CMake error: Imported target "agbpack" includes non-existent path "/tmp/include" in its INTERFACE_INCLUDE_DIRECTORIES.
#         I am not sure this ever worked. We may have always installed into /usr, where /usr/include most likely always existed.
#         * Or maybe we broke it when messing around with clownlzss_objects. Maybe undo that and use generator expressions again
#           * It complains about INTERFACE_INCLUDE_DIRECTORIES, so hopefully it's that
#         * Well actually it looks like we should remove the INCLUDES thing from InstallAgbpack.cmake
#           * We do not actually install any INCLUDES, so the directory will not be created
#           * Maybe before removing it we could do this experiment
#             * Build and install a faulty version into a clean tmp directory, copy that somewhere
#             * Fix the problem, install again
#             * Compare the two versions of the tmp directory (the install directoy)
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
add_library(agbpack::agbpack ALIAS agbpack)
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

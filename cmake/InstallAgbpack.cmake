# SPDX-FileCopyrightText: 2025 Thomas Mathys
# SPDX-License-Identifier: MIT

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Install agbpack module and exports
# Based on https://crascit.com/2024/04/04/cxx-modules-cmake-shared-libraries/#h-installing-shared-libraries-with-c-20-modules
install(
  TARGETS agbpack
  EXPORT agbpackTargets
  FILE_SET CXX_MODULES
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/agbpack/src
  FILE_SET HEADERS
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  INCLUDES
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(
  EXPORT agbpackTargets
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/agbpack
  CXX_MODULES_DIRECTORY .)

# Generate and install agbpackConfigVersion.cmake
write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/agbpackConfigVersion.cmake"
  COMPATIBILITY SameMajorVersion)
install(
  FILES
  "${PROJECT_BINARY_DIR}/agbpackConfigVersion.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/agbpack)

# Generate and install agbpackConfig.cmake
configure_package_config_file(
  "${PROJECT_SOURCE_DIR}/cmake/agbpackConfig.cmake.in"
  "${PROJECT_BINARY_DIR}/agbpackConfig.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/agbpack)
install(
  FILES
  "${PROJECT_BINARY_DIR}/agbpackConfig.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/agbpack)

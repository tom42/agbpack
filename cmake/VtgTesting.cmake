# SPDX-FileCopyrightText: 2024 Thomas Mathys
# SPDX-License-Identifier: MIT

function(enable_testing_with_catch2 version)
  enable_testing()

  # Try installed Catch2 package first.
  find_package(Catch2 ${version} QUIET)

  # If Catch2 package is not installed, get it using FetchContent.
  if(Catch2_FOUND)
    message(STATUS "Catch2 ${version} found")
  else()
    message(STATUS "Catch2 ${version} not found. Will obtain it using FetchContent")
    include(FetchContent)
    FetchContent_Declare(
      Catch2
      GIT_REPOSITORY https://github.com/catchorg/Catch2.git
      GIT_TAG        v${version}
    )
    FetchContent_MakeAvailable(Catch2)

    # Make Catch.cmake and CatchShardTests.cmake available.
    list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")
  endif()

  include(CTest)

  # Include Catch.cmake to get the catch_discover_tests function.
  include(Catch)

endfunction()

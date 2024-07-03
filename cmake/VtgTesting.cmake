# SPDX-FileCopyrightText: 2024 Thomas Mathys
# SPDX-License-Identifier: MIT

function(enable_testing_with_catch2 version)
  enable_testing()
  find_package(Catch2 ${version})

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
  endif()

endfunction()

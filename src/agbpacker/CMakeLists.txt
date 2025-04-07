# SPDX-FileCopyrightText: 2025 Thomas Mathys
# SPDX-License-Identifier: MIT

# TODO: not sure whether this is good: we have already raised warning levels
#       * Should we do this in the top level CMakeLists.txt before we've raised warning levels?
#       * Should we change the argpppp build process accordingly, since it also uses FetchContent to get argpppp?
# TODO: this adds argpppp tests to the build process. This is something we do not want to be done by default, but needs fixing on argpppp's side
include(FetchContent)
FetchContent_Declare(
  argpppp
  GIT_REPOSITORY https://github.com/tom42/argpppp
  GIT_TAG        v0.0.1)
FetchContent_MakeAvailable(argpppp)

add_executable(agbpacker agbpacker.cpp)
target_link_libraries(agbpacker PRIVATE agbpack argpppp)
install(TARGETS agbpacker DESTINATION bin)

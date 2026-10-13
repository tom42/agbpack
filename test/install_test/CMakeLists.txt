# SPDX-FileCopyrightText: 2026 Thomas Mathys
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.31.5)
project(agbpack_install_test LANGUAGES CXX)

if(NOT PROJECT_IS_TOP_LEVEL)
  message(FATAL_ERROR "This project is used to check whether agbpack was installed successfully. It should not be included in other projects.")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(agbpack REQUIRED)

# TODO: link to agbpack. What is the proper way to do so? Should we use a namespaced target here? (perhaps, but need to provide one first)
add_executable(agbpack_install_test agbpack_install_test.cpp)
target_link_libraries(agbpack_install_test agbpack)

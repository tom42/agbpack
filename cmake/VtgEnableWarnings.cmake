# SPDX-FileCopyrightText: 2024 Thomas Mathys
# SPDX-License-Identifier: MIT

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  # Using -Weverything is not recommended by clang developers, see
  # https://quuxplusone.github.io/blog/2018/12/06/dont-use-weverything
  # We start out with it anyway and see how it goes.
  add_compile_options(-Werror)
  add_compile_options(-Weverything)
  add_compile_options(-Wno-c++98-compat)
  add_compile_options(-Wno-c++98-compat-bind-to-temporary-copy)
  add_compile_options(-Wno-c++98-compat-pedantic)
  add_compile_options(-Wno-padded)
  add_compile_options(-Wno-switch-default)
  add_compile_options(-Wno-weak-vtables)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  # General warning options (https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html)
  add_compile_options(-Werror)
  add_compile_options(-Wall)
  add_compile_options(-Wextra)
  add_compile_options(-Wpedantic)
  add_compile_options(-Walloca)
  add_compile_options(-Wcast-align=strict)
  add_compile_options(-Wcast-qual)
  add_compile_options(-Wdisabled-optimization)
  add_compile_options(-Wduplicated-branches)
  add_compile_options(-Wduplicated-cond)
  add_compile_options(-Wformat=2)
  add_compile_options(-Winit-self)
  add_compile_options(-Winvalid-pch)
  add_compile_options(-Wlogical-op)
  add_compile_options(-Wmissing-declarations)
  add_compile_options(-Wmissing-include-dirs)
  add_compile_options(-Wnull-dereference)
  add_compile_options(-Wpointer-arith)
  add_compile_options(-Wredundant-decls)
  add_compile_options(-Wshadow)
  add_compile_options(-Wsign-conversion)
  add_compile_options(-Wswitch-enum)
  add_compile_options(-Wundef)
  add_compile_options(-Wwrite-strings)
  # C++ dialect options (https://gcc.gnu.org/onlinedocs/gcc/C_002b_002b-Dialect-Options.html)
  add_compile_options(-Wctor-dtor-privacy)
  add_compile_options(-Wnoexcept)
  add_compile_options(-Wold-style-cast)
  add_compile_options(-Wstrict-null-sentinel)
  add_compile_options(-Wsuggest-override)
  add_compile_options(-Wunsafe-loop-optimizations)
  add_compile_options(-Wuseless-cast)
  add_compile_options(-Wzero-as-null-pointer-constant)
endif()

if(MSVC)
  add_compile_options(/WX)
  add_compile_options(/W4)
endif()

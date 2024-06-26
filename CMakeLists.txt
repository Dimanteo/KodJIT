cmake_minimum_required(VERSION 3.0.0)
project(kodjit)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

find_package(GTest)
include(GoogleTest)
enable_testing()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CXX_SANITIZERS
    -fsanitize=address
    -fsanitize=alignment
    -fsanitize=bool
    -fsanitize=bounds
    -fsanitize=enum
    -fsanitize=float-cast-overflow
    -fsanitize=float-divide-by-zero
    -fsanitize=integer-divide-by-zero
    -fsanitize=leak
    -fsanitize=nonnull-attribute
    -fsanitize=null
    -fsanitize=return
    -fsanitize=returns-nonnull-attribute
    -fsanitize=shift
    -fsanitize=signed-integer-overflow
    -fsanitize=undefined
    -fsanitize=unreachable
    -fsanitize=vla-bound
    -fsanitize=vptr
    -fsized-deallocation)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  # add_compile_options(${CXX_SANITIZERS})
  # add_link_options(${CXX_SANITIZERS})
endif()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

add_subdirectory(lib)
add_subdirectory(test)

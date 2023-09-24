cmake_minimum_required(VERSION 3.0.0)
project(kodjit)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

find_package(GTest)
include(GoogleTest)
enable_testing()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

add_subdirectory(lib)
add_subdirectory(test)

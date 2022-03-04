cmake_minimum_required(VERSION 3.17)
project(LDB VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

# Alias to simplify the build down the road
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(Qt6 REQUIRED COMPONENTS Widgets Charts)
find_package(Threads REQUIRED)
find_package(TBB REQUIRED)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# TODO: Setup code coverage here
# maybe_enable_code_coverage()

add_subdirectory(resources)
add_subdirectory(extern)

# Main source directory
add_subdirectory(src)

# Source directory for applications that uses libary build from src
add_subdirectory(apps)

# TODO: Setup doxygen for documentation generation
add_subdirectory(docs)

# TODO: Setup the tests system and directory
enable_testing()
add_subdirectory(test)

cmake_minimum_required(VERSION 3.17)
project(LDB VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

# Alias to simplify the build down the road
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(Qt6 REQUIRED COMPONENTS Widgets Charts)
find_package(Threads REQUIRED)
find_package(TBB REQUIRED)
find_package(Boost REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
message(STATUS "CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")
find_package(LibDwarf REQUIRED)
find_package(Libunwind REQUIRED)
message(STATUS ${LIBUNWIND_LIBRARIES})
message(STATUS ${LIBDWARF_LIBRARIES})
message(STATUS ${LIBELF_LIBRARIES})

# TODO: Setup code coverage here
# maybe_enable_code_coverage()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

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

cmake_minimum_required(VERSION 3.17)
project(LDB VERSION 0.0.1 LANGUAGES C CXX)

# Alias to simplify the build down the road
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_subdirectory(src)

# Setup doxygen for documentation generation
add_subdirectory(docs)

# Setup the tests system and directory
enable_testing()
add_subdirectory(test)

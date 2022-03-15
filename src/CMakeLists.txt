

add_subdirectory(tracing)

add_subdirectory(gui)

set(CURRENT_INCLUDE_DIR ${INCLUDE_DIR})
add_library(ldb_app ldbapp.cpp)
target_include_directories(ldb_app PUBLIC ${CURRENT_INCLUDE_DIR})
target_link_libraries(ldb_app PUBLIC gui)
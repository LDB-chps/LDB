

add_executable(ldb ldb.cpp)
set_target_properties(ldb PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
target_link_libraries(ldb ldb_app)
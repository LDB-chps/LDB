

qt_add_executable(ldb ldb.cpp ${CMAKE_BINARY_DIR}/resources/icons.qrc)
set_target_properties(ldb PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
target_link_libraries(ldb PUBLIC ldb_app tbb)

add_executable(readDebugInfo readDebugInfo.cpp)
set_target_properties(readDebugInfo PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
target_link_libraries(readDebugInfo PUBLIC tracing)
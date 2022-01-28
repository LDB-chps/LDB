

add_subdirectory(tracing)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_subdirectory(gui)

set(CURRENT_INCLUDE_DIR ${INCLUDE_DIR})
qt_add_library(ldb_app ldbapp.cpp ${CMAKE_BINARY_DIR}/resources/icons.qrc)
target_include_directories(ldb_app PUBLIC ${CURRENT_INCLUDE_DIR})
target_link_libraries(ldb_app PUBLIC gui)
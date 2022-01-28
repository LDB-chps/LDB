

add_subdirectory(ldb)


set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CURRENT_INCLUDE_DIR ${INCLUDE_DIR})
add_library(ldb_app ldbapp.cpp ${CURRENT_INCLUDE_DIR}/ldbapp.h)
target_include_directories(ldb_app PUBLIC ${CURRENT_INCLUDE_DIR})
target_link_libraries(ldb_app PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Charts)
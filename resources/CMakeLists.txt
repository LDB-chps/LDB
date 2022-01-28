

find_package(Python REQUIRED)
execute_process(COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/icon-qrc-generator.py "${CMAKE_CURRENT_SOURCE_DIR}")
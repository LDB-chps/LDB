

find_package(Doxygen)
if (DOXYGEN_FOUND)
  set(doxyfile ${CMAKE_CURRENT_SOURCE_DIR}/doxyfile)
  add_custom_target(doc ALL
          ${DOXYGEN_EXECUTABLE} ${doxyfile}
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
          COMMENT "Running doxygen" VERBATIM)
endif ()
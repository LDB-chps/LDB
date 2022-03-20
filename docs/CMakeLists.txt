

find_package(Doxygen)
if (DOXYGEN_FOUND)
  set(doxyfile.in ${CMAKE_CURRENT_SOURCE_DIR}/doxyfile.in)
  set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/doxyfile)
  configure_file(
          ${doxyfile.in}
          ${doxyfile}
  )
  add_custom_target(doc ALL
          ${DOXYGEN_EXECUTABLE} ${doxyfile}
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
          COMMENT "Running doxygen" VERBATIM)
endif ()
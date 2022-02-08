
set(CURRENT_INCLUDE_DIR ${INCLUDE_DIR}/tracing)
qt_add_library(tracing STATIC
        process.cpp ${CURRENT_INCLUDE_DIR}/process.h
        processTracer.cpp ${CURRENT_INCLUDE_DIR}/processTracer.h)
target_include_directories(tracing PUBLIC ${CURRENT_INCLUDE_DIR})
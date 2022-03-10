
set(CURRENT_INCLUDE_DIR ${INCLUDE_DIR}/tracing)
qt_add_library(tracing STATIC
        process.cpp ${CURRENT_INCLUDE_DIR}/process.h
        processTracer.cpp ${CURRENT_INCLUDE_DIR}/processTracer.h
        registersSnapshot.cpp ${CURRENT_INCLUDE_DIR}/registersSnapshot.h

        DebugInfo.cpp ${CURRENT_INCLUDE_DIR}/DebugInfo.h

        # Elf related
        ELFReader.cpp ${CURRENT_INCLUDE_DIR}/ELFReader.h

        Section.cpp ${CURRENT_INCLUDE_DIR}/Section.h
        EHFrameReader.cpp ${CURRENT_INCLUDE_DIR}/EHFrameReader.h

        FrameList.cpp ${CURRENT_INCLUDE_DIR}/FrameList.h
        FrameTable.cpp ${CURRENT_INCLUDE_DIR}/FrameTable.h

        Symbol.cpp ${CURRENT_INCLUDE_DIR}/Symbol.h
        SymbolTable.cpp ${CURRENT_INCLUDE_DIR}/SymbolTable.h
        )
target_include_directories(tracing PUBLIC ${CURRENT_INCLUDE_DIR})
target_link_libraries(tracing PUBLIC TBB::tbb Threads::Threads)
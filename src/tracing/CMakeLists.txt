
set(CURRENT_INCLUDE_DIR ${INCLUDE_DIR}/tracing)
qt_add_library(tracing STATIC
        Process.cpp ${CURRENT_INCLUDE_DIR}/Process.h
        ProcessTracer.cpp ${CURRENT_INCLUDE_DIR}/ProcessTracer.h
        RegistersSnapshot.cpp ${CURRENT_INCLUDE_DIR}/RegistersSnapshot.h

        DebugInfo.cpp ${CURRENT_INCLUDE_DIR}/DebugInfo.h

        # Elf related
        ELFParser.cpp ${CURRENT_INCLUDE_DIR}/ELFParser.h

        Symbol.cpp ${CURRENT_INCLUDE_DIR}/Symbol.h
        SymbolTable.cpp ${CURRENT_INCLUDE_DIR}/SymbolTable.h

        DwarfReader.cpp ${CURRENT_INCLUDE_DIR}/DwarfReader.h
        StackFrame.cpp ${CURRENT_INCLUDE_DIR}/StackFrame.h
        StackTrace.cpp ${CURRENT_INCLUDE_DIR}/StackTrace.h
        SignalHandler.cpp ${CURRENT_INCLUDE_DIR}/SignalHandler.h

        BreakPointTable.cpp ${CURRENT_INCLUDE_DIR}/BreakPointTable.h
        BreakPointHandler.cpp ${CURRENT_INCLUDE_DIR}/BreakPointHandler.h
        )
target_include_directories(tracing PUBLIC ${CURRENT_INCLUDE_DIR})
target_link_libraries(tracing PUBLIC tscl::tscl TBB::tbb Threads::Threads ${LIBDWARF_LIBRARIES} ${LIBELF_LIBRARIES}
        ${LIBUNWIND_LIBRARIES})
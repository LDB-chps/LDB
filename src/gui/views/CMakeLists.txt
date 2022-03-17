
set(CURRENT_INCLUDE_DIR ${INCLUDE_DIR}/gui/views)
add_library(views
        VariableView.cpp ${CURRENT_INCLUDE_DIR}/VariableView.h
        LibraryView.cpp ${CURRENT_INCLUDE_DIR}/LibraryView.h
        TracerToolBar.cpp ${CURRENT_INCLUDE_DIR}/TracerToolBar.h
        StackTraceView.cpp ${CURRENT_INCLUDE_DIR}/StackTraceView.h
        CodeView.cpp ${CURRENT_INCLUDE_DIR}/CodeView.h
        ObjdumpView.cpp ${CURRENT_INCLUDE_DIR}/ObjdumpView.h
        SourceCodeView.cpp ${CURRENT_INCLUDE_DIR}/SourceCodeView.h
        BreakpointsDialog.cpp ${CURRENT_INCLUDE_DIR}/BreakpointsDialog.h
        )
target_link_libraries(views PUBLIC tracing Qt6::Core Qt6::Gui Qt6::Widgets)
target_include_directories(views PUBLIC ${INCLUDE_DIR} ${CURRENT_INCLUDE_DIR})
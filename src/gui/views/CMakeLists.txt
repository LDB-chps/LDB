
set(CURRENT_INCLUDE_DIR ${INCLUDE_DIR}/gui/views)
qt_add_library(views
        variableView.cpp ${CURRENT_INCLUDE_DIR}/variableView.h
        tracerToolBar.cpp ${CURRENT_INCLUDE_DIR}/tracerToolBar.h
        stackTraceView.cpp ${CURRENT_INCLUDE_DIR}/stackTraceView.h
        codeView.cpp ${CURRENT_INCLUDE_DIR}/codeView.h
        )
target_link_libraries(views PUBLIC tracing Qt6::Core Qt6::Gui Qt6::Widgets)
target_include_directories(views PUBLIC ${INCLUDE_DIR} ${CURRENT_INCLUDE_DIR})
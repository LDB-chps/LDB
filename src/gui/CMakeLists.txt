
set(CURRENT_INCLUDE_DIR ${INCLUDE_DIR}/gui)
qt_add_library(gui
        mainWindow.cpp ${CURRENT_INCLUDE_DIR}/mainWindow.h
        tracerPanel.cpp ${CURRENT_INCLUDE_DIR}/tracerPanel.h
        variableView.cpp ${CURRENT_INCLUDE_DIR}/variableView.h
        )
target_include_directories(gui PUBLIC ${CURRENT_INCLUDE_DIR})
target_link_libraries(gui PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Charts)

add_subdirectory(views)
add_subdirectory(logging)

set(CURRENT_INCLUDE_DIR ${INCLUDE_DIR}/gui)
add_library(gui
        MainWindow.cpp ${CURRENT_INCLUDE_DIR}/MainWindow.h
        TracerPanel.cpp ${CURRENT_INCLUDE_DIR}/TracerPanel.h
        CommandDialog.cpp ${CURRENT_INCLUDE_DIR}/CommandDialog.h
        PtyHandler.cpp ${CURRENT_INCLUDE_DIR}/PtyHandler.h
        CodeDisplay.cpp ${CURRENT_INCLUDE_DIR}/CodeDisplay.h
        QtSignalHandler.cpp ${CURRENT_INCLUDE_DIR}/QtSignalHandler.h
        )
target_include_directories(gui PUBLIC ${CURRENT_INCLUDE_DIR})
target_link_libraries(gui PUBLIC views logging tracing Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Charts)

set(CURRENT_INCLUDE_DIR ${INCLUDE_DIR}/gui/logging)
add_library(logging
        qtLogHandler.cpp ${CURRENT_INCLUDE_DIR}/qtLogHandler.h
        logWidget.cpp ${CURRENT_INCLUDE_DIR}/logWidget.h
        )
target_link_libraries(logging PUBLIC tscl::tscl Qt6::Core Qt6::Gui Qt6::Widgets)
target_include_directories(logging PUBLIC ${CURRENT_INCLUDE_DIR})
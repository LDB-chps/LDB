#pragma once
#include <tscl.hpp>


namespace ldb::gui {

  class LogWidget;

  class QtLogHandler : public tscl::LogHandler {
  public:
    QtLogHandler();
    void log(tscl::Log const& log, std::string const& message) override;

    LogWidget* getWidget();
    // Remove the log widget and disable this handler logging capabilities
    // This is useful since Qt might destroy the widget before we do
    void removeWidget();

  private:
    // Widget reponsible for displaying the log messages
    // note that Qt is reponsible for deleting this object
    LogWidget* log_widget;
  };

}// namespace ldb::gui

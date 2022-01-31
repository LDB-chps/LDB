#pragma once
#include <tscl.hpp>


namespace ldb::gui {

  class LogWidget;

  /**
   * @brief LogHandler specialization for Qt
   */
  class QtLogHandler : public tscl::LogHandler {
  public:
    QtLogHandler();
    /**
     * @brief Logs a message to the log widget
     * @param log Associated log
     * @param message the string containing the log's message
     */
    void log(tscl::Log const& log, std::string const& message) override;

    /**
     * @brief Returns the log widget associated with this handler
     * @return
     */
    LogWidget* getWidget();
    /**
     * @brief Remove the log widget and disable this handler logging capabilities
     * This is useful since Qt might destroy the widget before we do
     */
    void removeWidget();

  private:
    // Widget responsible for displaying the log messages
    // note that Qt is responsible for deleting this object
    LogWidget* log_widget;
  };

}// namespace ldb::gui

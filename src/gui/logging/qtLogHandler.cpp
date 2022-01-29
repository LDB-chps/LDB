#include "qtLogHandler.h"
#include "logWidget.h"

namespace ldb::gui {

  QtLogHandler::QtLogHandler() {
    log_widget = new LogWidget(this);
  }

  void QtLogHandler::log(tscl::Log const& log, std::string const& message) {
    std::scoped_lock<std::shared_mutex> lock(m_main_mutex);

    if (log_widget) { log_widget->log(log, message); }
  }

  LogWidget* QtLogHandler::getWidget() {
    std::shared_lock<std::shared_mutex> lock(m_main_mutex);
    return log_widget;
  }

  void QtLogHandler::removeWidget() {
    std::scoped_lock<std::shared_mutex> lock(m_main_mutex);
    log_widget = nullptr;
  }
}// namespace ldb::gui
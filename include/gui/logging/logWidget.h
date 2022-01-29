#pragma once
#include "qtLogHandler.h"
#include <QTextEdit>
#include <map>

namespace ldb::gui {

  class LogWidget : public QWidget {
  public:
    LogWidget(QtLogHandler* handler, QWidget* parent = nullptr);
    ~LogWidget();

    void log(const tscl::Log& log, const std::string& message);

  private:
    QtLogHandler* parent_handler;
    QTextEdit* text_edit;

    /**
     * @brief Associated colours for each log level
     *
     */
    std::map<tscl::Log::log_level, QString> color_map = {
            {tscl::Log::Trace, "#84818c"},       {tscl::Log::Debug, "#2aa1b3"},
            {tscl::Log::Information, "#ffffff"}, {tscl::Log::Warning, "#c4a000"},
            {tscl::Log::Error, "#c01c28"},       {tscl::Log::Fatal, "#a347ba"},
    };
  };
}// namespace ldb::gui

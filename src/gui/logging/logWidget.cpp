#include "logWidget.h"
#include <QGridLayout>

namespace ldb::gui {
  LogWidget::LogWidget(QtLogHandler* handler, QWidget* parent)
      : QFrame(parent), parent_handler(handler) {
    QGridLayout* layout = new QGridLayout(this);
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);

    text_edit = new QTextEdit;
    text_edit->setReadOnly(true);
    text_edit->setFontPointSize(11);
    text_edit->document()->setMaximumBlockCount(250);
    layout->addWidget(text_edit);
  }

  LogWidget::~LogWidget() {
    parent_handler->removeWidget();
  }

  void LogWidget::log(const tscl::Log& log, const std::string& message) {
    if (not parent_handler->enable() or log.level() < parent_handler->minLvl()) return;

    std::stringstream ss;

    ss << log.prefix(parent_handler->tsType()) << message;

    text_edit->setTextColor(QColor(color_map[log.level()]));
    text_edit->append(QString::fromStdString(ss.str()));
    text_edit->ensureCursorVisible();
  }
}// namespace ldb::gui
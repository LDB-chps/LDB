#include "tracerToolBar.h"
#include <QLabel>

namespace ldb::gui {
  TracerToolBar::TracerToolBar(QWidget* parent) : QToolBar(parent) {
    // By default, icons are a bit too large, reduce them
    setIconSize(QSize(16, 16));

    // Open a new program
    addAction(QIcon(":/icons/folder-open-fill.png"), "Open");
    addSeparator();

    // Program execution section
    addWidget(new QLabel("Program: <placeholder>"));
    addAction(QIcon(":/icons/skip-back-fill.png"), "Reset");
    addAction(QIcon(":/icons/play-fill.png"), "Play");
    addAction(QIcon(":/icons/stop-fill.png"), "Stop");
    addAction(QIcon(":/icons/skip-forward-fill.png"), "Finish");
    addSeparator();

    // Display section for current program
    addWidget(new QLabel("Current file: <placeholder>"));
    addWidget(new QLabel("PID: <placeholder>"));
  }
}// namespace ldb::gui
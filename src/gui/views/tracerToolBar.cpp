#include "tracerToolBar.h"
#include <QLabel>

namespace ldb::gui {
  TracerToolBar::TracerToolBar(QWidget* parent) : QToolBar(parent) {
    // By default, icons are a bit too large, reduce them
    setIconSize(QSize(16, 16));

    // Open a new program
    action_open_folder = new QAction(QIcon(":/icons/folder-open-fill.png"), "Open folder");
    addAction(action_open_folder);
    addSeparator();

    // Program execution section
    label_program_name = new QLabel("Program: ");
    addWidget(label_program_name);

    action_reset = new QAction(QIcon(":/icons/skip-back-fill.png"), "Reset");
    action_reset->setEnabled(false);
    addAction(action_reset);

    action_toggle_play = new QAction(QIcon(":/icons/play-fill.png"), "Play");
    action_toggle_play->setEnabled(false);
    addAction(action_toggle_play);

    action_stop = new QAction(QIcon(":/icons/stop-fill.png"), "Stop");
    action_stop->setEnabled(false);
    addAction(action_stop);

    action_continue = new QAction(QIcon(":/icons/skip-forward-fill.png"), "Continue");
    action_continue->setEnabled(false);
    addAction(action_continue);
    addSeparator();

    // Display section for current program
    addWidget(new QLabel("PID: "));
    addWidget(new QLabel("Current file: "));
  }
}// namespace ldb::gui
#include "tracerToolBar.h"
#include "gui/tracerPanel.h"
#include <QLabel>

namespace ldb::gui {
  TracerToolBar::TracerToolBar(TracerPanel* parent) : TracerView(parent), QToolBar(parent) {
    // By default, icons are a bit too large, reduce them
    setIconSize(QSize(20, 20));

    // Open a new program
    action_open_folder = new QAction(QIcon(":/icons/folder-open-fill.png"), "Start command");
    connect(action_open_folder, &QAction::triggered, this, &TracerToolBar::openCommand);
    addAction(action_open_folder);
    addSeparator();

    // Program execution section
    label_program_name = new QLabel("Program: ");
    addWidget(label_program_name);

    action_reset = new QAction(QIcon(":/icons/skip-back-fill.png"), "Reset");
    // action_reset->setEnabled(false);
    addAction(action_reset);

    action_toggle_play = new QAction(QIcon(":/icons/play-fill.png"), "Play");
    // action_toggle_play->setEnabled(false);
    addAction(action_toggle_play);

    action_stop = new QAction(QIcon(":/icons/stop-fill.png"), "Stop");
    // action_stop->setEnabled(false);
    addAction(action_stop);

    action_continue = new QAction(QIcon(":/icons/skip-forward-fill.png"), "Continue");
    // action_continue->setEnabled(false);
    addAction(action_continue);
    addSeparator();

    // Display section for current program
    addWidget(new QLabel("PID: "));
    connect(parent, &TracerPanel::tracerUpdated, this, &TracerToolBar::updateView);
    connect(parent, &TracerPanel::tracerUpdated, this, &TracerToolBar::startView);
  }

  void TracerToolBar::startView() {
    label_program_name->setText("Program: " +
                                QString::fromStdString(tracer_panel->getTracer()->getExecutable()));

  }

  void TracerToolBar::updateView() {}
}// namespace ldb::gui
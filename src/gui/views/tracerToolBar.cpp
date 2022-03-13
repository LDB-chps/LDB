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

    // Program execution section
    label_program_name = new QLabel("Program: ");
    addWidget(label_program_name);

    action_reset = new QAction(QIcon(":/icons/skip-back-fill.png"), "Reset");
    action_reset->setEnabled(false);
    // action_reset->setEnabled(false);
    addAction(action_reset);

    action_toggle_play = new QAction(QIcon(":/icons/play-fill.png"), "Play");
    action_toggle_play->setEnabled(false);
    // action_toggle_play->setEnabled(false);
    addAction(action_toggle_play);

    action_stop = new QAction(QIcon(":/icons/stop-fill.png"), "Stop");
    action_stop->setEnabled(false);
    // action_stop->setEnabled(false);
    addAction(action_stop);

    action_continue = new QAction(QIcon(":/icons/skip-forward-fill.png"), "Continue");
    action_continue->setEnabled(false);
    // action_continue->setEnabled(false);
    addAction(action_continue);

    label_pid = new QLabel("PID: ");
    addWidget(label_pid);

    connect(parent, &TracerPanel::tracerUpdated, this, &TracerToolBar::updateView);
    connect(parent, &TracerPanel::executionStarted, this, &TracerToolBar::startView);
  }

  void TracerToolBar::startView() {
    label_program_name->setText("Program: " +
                                QString::fromStdString(tracer_panel->getTracer()->getExecutable()));
    label_pid->setText("PID: " + QString::number(tracer_panel->getTracer()->getPid()));

    updateButtons();
  }

  void TracerToolBar::updateButtons() {
    auto status = tracer_panel->getTracer()->getProcessStatus();
    if (status == Process::Status::kRunning) {
      action_toggle_play->setIcon(QIcon(":/icons/pause-fill.png"));
    } else {
      action_toggle_play->setIcon(QIcon(":/icons/play-fill.png"));
    }

    if (status != Process::Status::kDead and status != Process::Status::kKilled) {
      action_toggle_play->setEnabled(true);
      action_reset->setEnabled(true);
      action_stop->setEnabled(true);
      action_continue->setEnabled(true);
    } else {
      action_toggle_play->setEnabled(false);
      action_reset->setEnabled(true);
      action_stop->setEnabled(false);
      action_continue->setEnabled(false);
    }
  }

  void TracerToolBar::updateView() {
    updateButtons();
  }
}// namespace ldb::gui
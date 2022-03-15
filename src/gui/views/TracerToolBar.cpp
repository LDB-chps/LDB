#include "TracerToolBar.h"
#include "gui/TracerPanel.h"
#include <QLabel>

namespace ldb::gui {
  TracerToolBar::TracerToolBar(TracerPanel* parent) : TracerView(parent), QToolBar(parent) {
    // By default, icons are a bit too large, reduce them
    setIconSize(QSize(16, 16));

    // Open a new program
    action_open_folder = new QAction(QIcon(":/icons/folder-open-fill.png"), "Start command");
    connect(action_open_folder, &QAction::triggered, parent, &TracerPanel::displayCommandDialog);
    addAction(action_open_folder);

    // Program execution section
    label_program_name = new QLabel("Program: ");
    addWidget(label_program_name);

    action_reset = new QAction(QIcon(":/icons/skip-back-fill.png"), "Reset");
    action_reset->setEnabled(false);
    // action_reset->setEnabled(false);
    addAction(action_reset);
    connect(action_reset, &QAction::triggered, parent, &TracerPanel::restartExecution);

    action_toggle_play = new QAction(QIcon(":/icons/play-fill.png"), "Play/Pause");
    action_toggle_play->setEnabled(false);
    // action_toggle_play->setEnabled(false);
    addAction(action_toggle_play);
    connect(action_toggle_play, &QAction::triggered, parent, &TracerPanel::toggleExecution);

    action_stop = new QAction(QIcon(":/icons/stop-fill.png"), "Stop");
    action_stop->setEnabled(false);
    // action_stop->setEnabled(false);
    addAction(action_stop);
    connect(action_stop, &QAction::triggered, parent, &TracerPanel::stopExecution);

    label_pid = new QLabel("PID: ");
    addWidget(label_pid);

    label_last_signal = new QLabel("");
    addWidget(label_last_signal);

    connect(parent, &TracerPanel::tracerUpdated, this, &TracerToolBar::updateView);
    connect(parent, &TracerPanel::executionEnded, this, &TracerToolBar::updateView);
    connect(parent, &TracerPanel::executionStarted, this, &TracerToolBar::startView);
  }

  void TracerToolBar::startView() {
    label_program_name->setText("Program: " +
                                QString::fromStdString(tracer_panel->getTracer()->getExecutable()));
    label_pid->setText("PID: " + QString::number(tracer_panel->getTracer()->getPid()));

    updateButtons();
  }

  void TracerToolBar::updateButtons() {

    auto status = Process::Status::kDead;
    auto ls = Signal::kUnknown;
    // Gather information about the current process
    if (tracer_panel->getTracer() != nullptr) {
      status = tracer_panel->getTracer()->getProcessStatus();
      ls = tracer_panel->getTracer()->getLastSignal();
    }

    if (status == Process::Status::kStopped) {
      action_toggle_play->setIcon(QIcon(":/icons/play-fill.png"));
    } else {
      action_toggle_play->setIcon(QIcon(":/icons/pause-fill.png"));
    }

    if (status != Process::Status::kDead and status != Process::Status::kKilled) {
      action_toggle_play->setEnabled(true);
      action_reset->setEnabled(true);
      action_stop->setEnabled(true);
    } else {
      action_toggle_play->setEnabled(false);
      action_reset->setEnabled(true);
      action_stop->setEnabled(false);
    }

    if (ls != Signal::kUnknown)
      label_last_signal->setText(
              "Last signal: " +
              QString::fromStdString(signalToString(tracer_panel->getTracer()->getLastSignal())));
    else
      label_last_signal->setText("");
  }

  void TracerToolBar::updateView() {
    updateButtons();
  }
}// namespace ldb::gui
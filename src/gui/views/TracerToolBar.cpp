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
    addSeparator();

    action_reset = new QAction(QIcon(":/icons/replay.png"), "Reset");
    action_reset->setEnabled(false);
    // action_reset->setEnabled(false);
    addAction(action_reset);
    connect(action_reset, &QAction::triggered, parent, &TracerPanel::restartExecution);

    action_toggle_play = new QAction(QIcon(":/icons/play-fill.png"), "Play/Pause");
    action_toggle_play->setEnabled(false);
    addAction(action_toggle_play);
    connect(action_toggle_play, &QAction::triggered, parent, &TracerPanel::toggleExecution);

    action_stop = new QAction(QIcon(":/icons/stop-fill.png"), "Stop");
    action_stop->setEnabled(false);
    // action_stop->setEnabled(false);
    addAction(action_stop);
    connect(action_stop, &QAction::triggered, parent, &TracerPanel::abortExecution);

    label_last_signal = new QLabel("");
    addWidget(label_last_signal);

    // Breakpoint section
    addSeparator();

    action_breakpoints = new QAction(QIcon(":/icons/breakpoint.png"), "Display breakpoints");
    connect(action_breakpoints, &QAction::triggered, parent, &TracerPanel::displayBreakpoints);
    action_breakpoints->setEnabled(false);
    addAction(action_breakpoints);

    action_step = new QAction(QIcon(":/icons/step.png"), "single-step");
    connect(action_step, &QAction::triggered, parent, &TracerPanel::singlestep);
    action_step->setEnabled(false);
    addAction(action_step);

    // Program execution section
    addSeparator();

    label_program_name = new QLabel("Program: ");
    addWidget(label_program_name);

    label_pid = new QLabel("PID: ");
    addWidget(label_pid);


    connect(parent, &TracerPanel::signalReceived, this, &TracerToolBar::updateView);
    connect(parent, &TracerPanel::executionEnded, this, &TracerToolBar::updateButtons);
    connect(parent, &TracerPanel::executionStarted, this, &TracerToolBar::startView);
  }

  void TracerToolBar::startView() {

    label_last_signal->setText("");
    auto* tracer = tracer_panel->getTracer();
    label_program_name->setText("Program: " + QString::fromStdString(tracer->getExecutable()));
    label_pid->setText("PID: " + QString::number(tracer->getProcess().getPid()));
    label_last_signal->setText("Waiting for program start");

    updateButtons();
  }

  void TracerToolBar::updateButtons() {

    auto status = Process::Status::kDead;
    auto ls = Signal::kUnknown;

    // Gather information about the current process
    if (tracer_panel->getTracer() != nullptr) {
      auto* tracer = tracer_panel->getTracer();
      status = tracer->getProcess().getStatus();
      action_breakpoints->setEnabled(true);
    }


    if (status == Process::Status::kStopped) {
      action_toggle_play->setIcon(QIcon(":/icons/play-fill.png"));
      action_step->setEnabled(true);
    } else {
      action_toggle_play->setIcon(QIcon(":/icons/pause-fill.png"));
      action_step->setEnabled(false);
    }

    if (status != Process::Status::kDead and status != Process::Status::kKilled and
        status != Process::Status::kExited) {
      action_toggle_play->setEnabled(true);
      action_reset->setEnabled(true);
      action_stop->setEnabled(true);
    } else {
      action_toggle_play->setEnabled(false);
      action_reset->setEnabled(true);
      action_stop->setEnabled(false);
    }
  }

  void TracerToolBar::updateView(SignalEvent evt) {
    updateButtons();
    if (evt.getStatus() == Process::Status::kExited) label_last_signal->setText("Exited");
    else
      label_last_signal->setText(QString::fromStdString(signalToString(evt.getSignal())));
  }
}// namespace ldb::gui
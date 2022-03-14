#include "tracerPanel.h"
#include "PtyHandler.h"
#include "commandDialog.h"
#include "logWidget.h"
#include <QMessageBox>
#include <QSplitter>
#include <QTabWidget>
#include <QTextEdit>
#include <QToolButton>
#include <boost/algorithm/string.hpp>
#include <tscl.hpp>

namespace ldb::gui {
  TracerPanel::TracerPanel(QWidget* parent) : QWidget(parent) {

    auto* layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setupToolbar(layout);

    // We want to be able to dynamically resize the terminal
    // For this, we need to create a splitter, and put both the terminal the the other widgets in it
    //   TOP_SIDE
    //   ============= SPLITTER
    //   TERMINAL_TABS
    auto* splitter = new QSplitter(Qt::Vertical, this);
    layout->addWidget(splitter, 1, 0);

    // Create a layout for the inside of the splitter
    auto* splitter_layout = new QGridLayout(nullptr);
    splitter_layout->setContentsMargins(0, 0, 0, 0);
    splitter_layout->setSpacing(0);

    // Create a dummy widget that will contain every top-side widget
    auto widget = new QWidget;
    widget->setLayout(splitter_layout);
    splitter->addWidget(widget);
    // Add the top side widget in the dummy
    setupVariableView(splitter_layout);
    setupCodeView(splitter_layout);

    // Setup the terminal tabs
    auto* tabbed_pane = setupTabbedPane();
    splitter->addWidget(tabbed_pane);

    // The terminal should be smaller than the top side
    // High stretch factor for the top side
    splitter->setStretchFactor(0, 6);
    // Low stretch factor for the terminal
    splitter->setStretchFactor(1, 3);
  }

  TracerPanel::~TracerPanel() {
    // Kill the tracer before exiting
    endTracer(true);
  }

  void TracerPanel::setupToolbar(QGridLayout* layout) {
    // Setup the main toolbar
    toolbar = new TracerToolBar(this);
    layout->addWidget(toolbar, 0, 0);
    connect(toolbar, &TracerToolBar::openCommand, this, &TracerPanel::displayCommandDialog);
  }

  void TracerPanel::TracerPanel::setupCodeView(QGridLayout* layout) {
    code_view = new CodeView(this);
    layout->addWidget(code_view, 0, 1, 1, 1);
  }

  void TracerPanel::setupVariableView(QGridLayout* layout) {
    variable_view = new VariableView(this);
    layout->addWidget(variable_view, 0, 0, 1, 1);
  }

  QTabWidget* TracerPanel::setupTabbedPane() {
    auto* stacked_pane = new QTabWidget(this);
    stacked_pane->setTabPosition(QTabWidget::North);
    stacked_pane->setTabShape(QTabWidget::Rounded);
    stacked_pane->setIconSize(QSize(16, 16));

    // Setup the tab where the log will be displayed
    // Create a QtLogHandler that will display the log in the log widget
    auto& logger = tscl::logger.addHandler<QtLogHandler>("QtHandler");
    logger.tsType(tscl::timestamp_t::Partial);

    // Widget associated with the logger
    auto message = logger.getWidget();

    stacked_pane->addTab(message, "Message");
    stacked_pane->setTabIcon(0, QIcon(":/icons/menu-2-line.png"));

    // Setup the tab where the process output and input will be displayed
    pty_handler = new PtyHandler(this, -1);
    stacked_pane->addTab(pty_handler, "Input/Output");
    stacked_pane->setTabIcon(1, QIcon(":/icons/terminal-box-fill.png"));

    // Setup the tab where the stack trace will be displayed
    stack_trace_view = new StackTraceView(this);
    stacked_pane->addTab(stack_trace_view, "Stack trace");
    stacked_pane->setTabIcon(2, QIcon(":/icons/stack-fill.png"));

    // Setup the tab where the loaded libraries will be displayed
    auto libs = new QTextEdit(this);
    stacked_pane->addTab(libs, "Loaded libraries");
    stacked_pane->setTabIcon(3, QIcon(":/icons/list-settings-line.png"));

    return stacked_pane;
  }

  void TracerPanel::setupThreads() {
    endThreads();
    if (not process_tracer) return;

    pty_handler->reassignTo(process_tracer->getMasterFd());
    update_thread = QThread::create(&TracerPanel::updateLoop, this);
    update_thread->start();
  }

  void TracerPanel::endThreads() {
    if (update_thread and update_thread->isRunning()) {
      update_thread->terminate();
      update_thread = nullptr;
    }
    pty_handler->reassignTo(-1);
  }

  void TracerPanel::toggleExecution() {
    if (not process_tracer) return;

    auto& p = process_tracer->getProcess();
    auto status = p.getStatus();
    if (status == Process::Status::kStopped) {
      p.resume();
      emit tracerUpdated();
    } else {
      p.pause();
    }
  }

  void TracerPanel::stopExecution() {

    if (not process_tracer or process_tracer->getProcessStatus() == Process::Status::kDead) return;

    auto res = QMessageBox::question(this, "End execution",
                                     "An execution is running.\nAre you sure you want to stop it ?",
                                     QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (res == QMessageBox::No) return;

    endThreads();
    auto& p = process_tracer->getProcess();
    p.kill();
    emit executionEnded();
  }

  void TracerPanel::displayCommandDialog() {
    auto* dialog = new CommandDialog(this);
    dialog->setModal(true);
    if (dialog->exec() != QDialog::Accepted) return;
    startExecution(dialog->getCommand().toStdString(), dialog->getArgs().toStdString());
  }

  void TracerPanel::restartExecution(bool force) {
    if (not process_tracer) return;

    if (not force) {
      auto res = QMessageBox::question(
              this, "Restart execution",
              "An execution is already started.\nAre you sure you want to restart it ?",
              QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
      if (res == QMessageBox::No) return;
    }

    endThreads();
    bool res = process_tracer->restart();

    if (not res) tscl::logger("Failed to restart the process.", tscl::Log::Error);
    else
      tscl::logger("Process restarted.", tscl::Log::Information);

    setupThreads();
    emit executionStarted();
  }

  bool TracerPanel::startExecution(const std::string& command, const std::string& args,
                                   bool force) {

    std::vector<std::string> args_vec;
    boost::split(args_vec, args, boost::is_any_of(" \n\t"));
    return startExecution(command, args_vec, force);
  }


  bool TracerPanel::startExecution(const std::string& command, const std::vector<std::string>& args,
                                   bool force) {

    if (process_tracer and not force) {
      auto res = QMessageBox::question(
              this, "Start execution",
              "An execution is already started.\nAre you sure you want to start a new one ?",
              QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
      if (res == QMessageBox::No) return false;
    }

    // Cleanup the current state
    if (process_tracer) { endTracer(true); }

    try {
      tscl::logger("Starting executable: " + command, tscl::Log::Information);
      process_tracer = ProcessTracer::fromCommand(command, args);
      if (not process_tracer) {
        tscl::logger("Failed to start executable", tscl::Log::Error);
        return false;
      }

      setupThreads();

      // Emit signals to update the UI accordingly
      emit executionStarted();
      emit tracerUpdated();
    } catch (const std::exception& e) {
      tscl::logger("Failed to start command: " + command + ":", tscl::Log::Error);
      tscl::logger(e.what(), tscl::Log::Error);
      return false;
    }
    return true;
  }

  void TracerPanel::endTracer(bool force) {

    if (not process_tracer) return;

    if (not force) {
      auto res = QMessageBox::question(
              this, "End execution", "An execution is running.\nAre you sure you want to end it ?",
              QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
      if (res == QMessageBox::No) return;
    }

    endThreads();

    // No need to manually kill the process tracer, it will end itself
    process_tracer = nullptr;

    emit executionEnded();
  }

  void TracerPanel::updateLoop() {
    bool done = false;
    while (not done) {
      if (not process_tracer) { done = true; }
      auto status = process_tracer->waitNextEvent();
      if (status == Process::Status::kDead or status == Process::Status::kKilled or
          status == Process::Status::kUnknown) {
        done = true;
      }
      emit tracerUpdated();
    }
  }
}// namespace ldb::gui

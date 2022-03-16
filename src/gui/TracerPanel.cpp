#include "TracerPanel.h"
#include "CommandDialog.h"
#include "LibraryView.h"
#include "PtyHandler.h"
#include "logWidget.h"
#include <QHBoxLayout>
#include <QMessageBox>
#include <QSplitter>
#include <QTabWidget>
#include <QTextEdit>
#include <QToolButton>
#include <QVBoxLayout>
#include <boost/algorithm/string.hpp>
#include <tscl.hpp>

namespace ldb::gui {
  TracerPanel::TracerPanel(QWidget* parent) : QWidget(parent) {

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    toolbar = new TracerToolBar(this);
    layout->addWidget(toolbar);


    // We want to be able to dynamically resize the panels
    // For this, we need to create a horizontal_splitter, and put both the terminal the the other
    // widgets in it
    //                    TOP_PANEL
    //   ============= HORIZONTAL SPLITTER ============
    //   MESSAGE PANEL         |  INFORMATION PANEL
    //                 VERTICAL splitter
    //                         |
    auto* horizontal_splitter = new QSplitter(Qt::Vertical, this);
    layout->addWidget(horizontal_splitter);

    // Create a layout for the inside of the horizontal_splitter
    auto* top_panel_layout = new QHBoxLayout(nullptr);
    top_panel_layout->setContentsMargins(0, 0, 0, 0);
    top_panel_layout->setSpacing(0);

    // Create a dummy dummy_widget that will contain every top-side dummy_widget
    auto top_widget = new QWidget;
    top_widget->setLayout(top_panel_layout);
    horizontal_splitter->addWidget(top_widget);

    code_view = new CodeView(this);
    top_panel_layout->addWidget(code_view);

    // Setup the bottom panel
    // MESSAGE PANEL || INFORMATION PANEL

    auto* vertical_splitter = new QSplitter(Qt::Horizontal, horizontal_splitter);
    horizontal_splitter->addWidget(vertical_splitter);

    // The bottom panel should be smaller than the top side
    // High stretch factor for the top side
    horizontal_splitter->setStretchFactor(0, 6);
    // Low stretch factor for the bottom side
    horizontal_splitter->setStretchFactor(1, 3);

    auto* message_tabs = new QTabWidget(vertical_splitter);
    message_tabs->setIconSize(QSize(16, 16));
    vertical_splitter->addWidget(message_tabs);

    // Setup the tab where the log will be displayed
    // Create a QtLogHandler that will display the log in the log widget
    auto& logger = tscl::logger.addHandler<QtLogHandler>("QtHandler");
    logger.tsType(tscl::timestamp_t::Partial);

    // Widget associated with the logger
    auto message = logger.getWidget();

    message_tabs->addTab(message, "Message");
    message_tabs->setTabIcon(0, QIcon(":/icons/menu-2-line.png"));

    // Setup the tab where the process output and input will be displayed
    pty_handler = new PtyHandler(this, -1);
    message_tabs->addTab(pty_handler, "Input/Output");
    message_tabs->setTabIcon(1, QIcon(":/icons/terminal-box-fill.png"));
    connect(this, &TracerPanel::executionStarted, [=]() { message_tabs->setCurrentIndex(1); });

    auto* information_tab = new QTabWidget(vertical_splitter);
    information_tab->setIconSize(QSize(16, 16));
    vertical_splitter->addWidget(information_tab);

    // Setup the tab where the stack trace will be displayed
    stack_trace_view = new StackTraceView(this);
    information_tab->addTab(stack_trace_view, "Stack trace");
    information_tab->setTabIcon(0, QIcon(":/icons/stack-fill.png"));

    variable_view = new VariableView(this);
    information_tab->addTab(variable_view, "Registers");
    information_tab->setTabIcon(1, QIcon(":/icons/view-module.png"));

    // Setup the tab where the loaded libraries will be displayed
    auto libs = new LibraryView(this);
    information_tab->addTab(libs, "Loaded libraries");
    information_tab->setTabIcon(2, QIcon(":/icons/list-settings-line.png"));
  }

  TracerPanel::~TracerPanel() {
    // Kill the tracer before exiting
    endTracer(true);
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
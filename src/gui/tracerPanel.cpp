#include "tracerPanel.h"
#include "commandDialog.h"
#include "logWidget.h"
#include <QLabel>
#include <QProgressBar>
#include <QSplitter>
#include <QTabWidget>
#include <QTextEdit>
#include <QToolBar>
#include <QToolButton>
#include <tscl.hpp>
#include "PtyHandler.h"

namespace ldb::gui {
  TracerPanel::TracerPanel(QWidget* parent) : QWidget(parent) {

    auto* layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setupToolbar(layout);

    auto* splitter = new QSplitter(Qt::Vertical, this);
    layout->addWidget(splitter, 1, 0);
    auto* splitter_layout = new QGridLayout(nullptr);
    splitter_layout->setContentsMargins(0, 0, 0, 0);
    splitter_layout->setSpacing(0);
    auto widget = new QWidget;
    widget->setLayout(splitter_layout);
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    splitter->addWidget(widget);
    setupVariableView(splitter_layout);
    setupCodeView(splitter_layout);

    auto* tabbed_pane = setupTabbedPane();
    splitter->addWidget(tabbed_pane);
    // The terminal should be smaller than the other two panes.
    splitter->setStretchFactor(0, 6);
    splitter->setStretchFactor(1, 3);
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
    stacked_pane->setTabPosition(QTabWidget::South);
    stacked_pane->setTabShape(QTabWidget::Rounded);
    stacked_pane->setIconSize(QSize(16, 16));

    // Setup the tab where the log will be displayed
    auto& logger = tscl::logger.addHandler<QtLogHandler>("QtHandler");
    logger.tsType(tscl::timestamp_t::Partial);
    auto message = logger.getWidget();

    stacked_pane->addTab(message, "Message");
    stacked_pane->setTabIcon(0, QIcon(":/icons/menu-2-line.png"));

    // Setup the tab where the process output and input will be displayed
    pipe_handler = new PtyHandler(this, -1);
    stacked_pane->addTab(pipe_handler, "Input/Output");
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

  void TracerPanel::displayCommandDialog() {
    auto* dialog = new CommandDialog(this);
    dialog->setModal(true);
    if (dialog->exec() != QDialog::Accepted) return;

    try {
      tscl::logger("Starting executable: " + dialog->getCommand().toStdString(),
                   tscl::Log::Information);
      process_tracer = ProcessTracer::fromCommand(dialog->getCommand().toStdString(),
                                                  dialog->getArgs().toStdString());
      if (not process_tracer) {
        tscl::logger("Failed to start executable", tscl::Log::Error);
        return;
      }

      // Redirect the process output to the dedicated widget
      pipe_handler->setPipes(process_tracer->getMasterFd());
      update_thread = QThread::create(&TracerPanel::updateLoop, this);
      update_thread->start();

      // Emit signals to update the UI accordingly
      emit executionStarted();
      emit tracerUpdated();
    } catch (const std::exception& e) {
      tscl::logger("Failed to start command: " + dialog->getCommand().toStdString() + ":",
                   tscl::Log::Error);
      tscl::logger(e.what(), tscl::Log::Error);
      return;
    }
  }

  void TracerPanel::updateLoop() {
    bool done = false;
    while (not done) {
      if (not process_tracer) {
        done = true;
      }
      auto status = process_tracer->waitNextEvent();
      if (status == Process::Status::kDead or status == Process::Status::kKilled) {
        done = true;
      }
      emit tracerUpdated();
    }
  }
}// namespace ldb::gui

#pragma once
#include <QApplication>
#include <QGridLayout>
#include <QThread>
#include <QWidget>
#include "BreakpointsDialog.h"
#include "ObjdumpView.h"
#include "ProcessTracer.h"
#include "PtyHandler.h"
#include "QtSignalHandler.h"
#include "SourceCodeView.h"
#include "StackTraceView.h"
#include "TracerToolBar.h"
#include "VariableView.h"

namespace ldb::gui {

  /**
   * @brief Main panel for the application
   * Contains multiple views for the different parts of the debugger
   * Also provides multiple actions to control the tracer
   *
   * This class emits signals when something happens to the tracee
   * Author note: This class is quite big and could be split up
   * into smaller classes. However, we don't want do deal with this for a project of this scale.
   */
  class TracerPanel : public QWidget {
    Q_OBJECT

  public:
    /**
     * @brief Builds a new TracerPanel and create blank views
     * @param parent
     */
    explicit TracerPanel(QWidget* parent = nullptr);
    ~TracerPanel() override;

    /**
     * @brief Returns the current tracer. May return nullptr
     * @return Returns the current tracer if any, nullptr otherwise
     */
    ProcessTracer* getTracer() {
      return process_tracer.get();
    }

  public slots:

    /**
     * @brief Toggle the current tracee between running and paused
     */
    void toggleExecution();

    /**
     * @brief Perform a single step in the tracee
     */
    void singlestep();

    /**
     * @brief Stop the process without killing the tracer
     */
    void abortExecution();

    /**
     * @brief Restart the same command with the same arguments
     * Note that the program may have changed (e.g. if it was recompiled)
     * @param force
     */
    void restartExecution(bool force = false);

    /**
     * @brief Creates a dialog to select a process to start tracing.
     */
    void displayCommandDialog();

    /**
     * @brief Display a dialog to select a breakpoint
     */
    void displayBreakpoints() {
      breakpoints_dialog->show();
    }

    /**
     * @brief Starts the tracer
     * @param command The command to start
     * @param args Arguments to pass to the command
     * @param force If true, the tracer will be restarted even if it is already running
     * Otherwise, a popup will let the user decide whether to restart or not
     * @return Truee if the execution was started, false otherwise
     */
    bool startExecution(const std::string& command, const std::string& args, bool force = false);

    /**
     * @brief Start a new program, killing the current one if any.
     * @param command The command to start
     * @param args The arguments to pass to the command
     */
    bool startExecution(const std::string& command, const std::vector<std::string>& args,
                        bool force = false);

    /**
     * @brief Stop the current program if any.
     */
    void endTracer(bool force = false);

  signals:

    /**
     * @brief Emitted when the tracer is started
     */
    void executionStarted();

    /**
     * @brief Emitted when the tracee changes status
     * For example, when it receives a signal, or its overal status is changed
     */
    void signalReceived(SignalEvent event);

    /**
     * @brief Emitted when the tracee stops
     */
    void executionEnded();

  private:
    std::unique_ptr<ProcessTracer> process_tracer;

    QThread* update_thread = nullptr;
    TracerToolBar* toolbar = nullptr;
    VariableView* variable_view = nullptr;
    StackTraceView* stack_trace_view = nullptr;
    ObjdumpView* objdump_view = nullptr;
    SourceCodeView* code_view = nullptr;
    PtyHandler* pty_handler = nullptr;
    BreakpointsDialog* breakpoints_dialog = nullptr;
  };
}// namespace ldb::gui

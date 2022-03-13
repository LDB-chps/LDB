#pragma once

#include "ProcessTracer.h"
#include "PtyHandler.h"
#include "codeView.h"
#include "stackTraceView.h"
#include "tracerToolBar.h"
#include "variableView.h"
#include <QGridLayout>
#include <QThread>
#include <QWidget>

namespace ldb::gui {

  /**
   * @brief Main panel for the application
   * Contains multiple views for the different parts of the debugger
   * Also provides multiple actions to control the tracer
   *
   * This class emits signals when something happens to the tracee
   */
  class TracerPanel : public QWidget {
    Q_OBJECT

  public:
    /**
     * @brief Builds a new TracerPanel and create blank views
     * @param parent
     */
    explicit TracerPanel(QWidget* parent = nullptr);

    /**
     * @brief Returns the current tracer. May return nullptr
     * @return Returns the current tracer if any, nullptr otherwise
     */
    ProcessTracer* getTracer() {
      return process_tracer.get();
    }


  public slots:

    /**
     * @brief Resume the current tracee if it is paused
     */
    void resumeExecution();

    /**
     * @brief Pause the current tracee if it is running
     */
    void pauseExecution();

    /**
     * @brief Toggle the current tracee between running and paused
     */
    void toggleExecution();

    void maybeRestartExecution();

    /**
     * @brief Attempt to start the command if no program is already running. Otherwise,
     * start a dialog to ask the user if he wants to stop the current program.
     * @param command The command to start
     * @param args The arguments to pass to the command
     */
    void maybeStartExecution(const std::string& command, const std::string& args);

    /**
     * @brief Creates a dialog to select a process to start tracing.
     */
    void displayCommandDialog();

    bool startExecution(const std::string& command, const std::string& args);

    /**
     * @brief Start a new program, killing the current one if any.
     * @param command The command to start
     * @param args The arguments to pass to the command
     */
    bool startExecution(const std::string& command, const std::vector<std::string>& args);

    /**
     * @brief Popup a dialog to ask the user if he wants to stop the current program if any.
     */
    void maybeEndExecution();

    /**
     * @brief Stop the current program if any.
     */
    void endExecution();

  signals:

    /**
     * @brief Emitted when the tracer is started
     */
    void executionStarted();

    /**
     * @brief Emitted when the tracee changes status
     * For example, when it receives a signal, or its overal status is changed
     */
    void tracerUpdated();

    /**
     * @brief Emitted when the tracee stops
     */
    void executionEnded();

  private:

    /**
     * @brief Loop function on which a new thread is created to periodically update the tracee status
     */
    void updateLoop();


    std::string old_cmd;
    std::vector<std::string> old_args;

    std::unique_ptr<ProcessTracer> process_tracer;

    void setupToolbar(QGridLayout* layout);

    void setupCodeView(QGridLayout* layout);

    void setupVariableView(QGridLayout* layout);

    QTabWidget* setupTabbedPane();

    QThread* update_thread = nullptr;
    TracerToolBar* toolbar = nullptr;
    VariableView* variable_view = nullptr;
    StackTraceView* stack_trace_view = nullptr;
    CodeView* code_view = nullptr;
    PtyHandler* pty_handler = nullptr;
  };
}// namespace ldb::gui

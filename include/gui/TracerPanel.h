#pragma once

#include "CodeView.h"
#include "ObjdumpView.h"
#include "ProcessTracer.h"
#include "PtyHandler.h"
#include "QtSignalHandler.h"
#include "StackTraceView.h"
#include "TracerToolBar.h"
#include "VariableView.h"
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
     * @brief Stop the process without killing the tracer
     */
    void abortExecution();

    void restartExecution(bool force = false);

    /**
     * @brief Creates a dialog to select a process to start tracing.
     */
    void displayCommandDialog();

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
    void setupToolbar(QGridLayout* layout);

    void setupCodeView(QGridLayout* layout);

    void setupVariableView(QGridLayout* layout);

    QTabWidget* setupTabbedPane();

    std::unique_ptr<ProcessTracer> process_tracer;

    QThread* update_thread = nullptr;
    TracerToolBar* toolbar = nullptr;
    VariableView* variable_view = nullptr;
    StackTraceView* stack_trace_view = nullptr;
    ObjdumpView* objdump_view = nullptr;
    CodeView* code_view = nullptr;
    PtyHandler* pty_handler = nullptr;
  };
}// namespace ldb::gui

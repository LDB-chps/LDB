#pragma once

#include "codeView.h"
#include "processTracer.h"
#include "stackTraceView.h"
#include "tracerToolBar.h"
#include "variableView.h"
#include <QGridLayout>
#include <QWidget>

namespace ldb::gui {

  class TracerPanel : public QWidget {
    Q_OBJECT

  public:
    explicit TracerPanel(QWidget* parent = nullptr);

    ProcessTracer* getTracer() {
      return process_tracer.get();
    }

  public slots:

    /**
     * @brief Creates a dialog to select a process to start tracing.
     */
    void displayCommandDialog();

    /**
     * @brief Attempt to start the command if no program is already running. Otherwise,
     * start a dialog to ask the user if he wants to stop the current program.
     * @param command The command to start
     * @param args The arguments to pass to the command
     */
    void maybeStartExecution(const std::string& command, const std::vector<std::string>& args) {}

    /**
     * @brief Start a new program, killing the current one if any.
     * @param command The command to start
     * @param args The arguments to pass to the command
     */
    bool startExecution(const std::string& command, const std::vector<std::string>& args) {
      return true;
    }

    /**
     * @brief Popup a dialog to ask the user if he wants to stop the current program if any.
     */
    void maybeEndExecution() {}

    /**
     * @brief Stop the current program if any.
     */
    void endExecution() {}

  signals:

    void tracerUpdated();
    void executionStarted();
    void executionEnded();
    void tracerRaisedError();

  private:
    std::unique_ptr<ProcessTracer> process_tracer;

    void setupToolbar(QGridLayout* layout);

    void setupCodeView(QGridLayout* layout);

    void setupVariableView(QGridLayout* layout);

    QTabWidget* setupTabbedPane();

    TracerToolBar* toolbar = nullptr;
    VariableView* variable_view = nullptr;
    StackTraceView* stack_trace_view = nullptr;
    CodeView* code_view = nullptr;
  };
}// namespace ldb::gui

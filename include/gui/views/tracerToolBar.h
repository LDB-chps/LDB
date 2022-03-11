#pragma once

#include "tracerView.h"
#include <QLabel>
#include <QToolBar>
#include <processTracer.h>

namespace ldb::gui {
  class TracerToolBar : public QToolBar {
    Q_OBJECT
  public:
    TracerToolBar(QWidget* parent = nullptr);

    /**
     * @brief Update the view to reflect the tracer state
     * @param tracer A pointer to the tracer. If nullptr, the view should present a blank state.
     */
    void update(ProcessTracer* tracer) {}

  signals:
    // Emitted when the open command button is clicked
    void openCommand();

  private:
    QAction* action_open_folder;
    QAction* action_toggle_play;
    QAction* action_stop;
    QAction* action_reset;
    QAction* action_continue;
    QLabel* label_program_name;
    QLabel* label_program_id;
    QLabel* label_current_file;
  };
}// namespace ldb::gui

#pragma once

#include "tracerView.h"
#include <QLabel>
#include <QToolBar>
#include <ProcessTracer.h>
#include "tracerView.h"

namespace ldb::gui {
  class TracerToolBar : public QToolBar, public TracerView {
    Q_OBJECT
  public:
    TracerToolBar(TracerPanel* parent = nullptr);


  public slots:
    void updateView();
    void startView();

  signals:
    // Emitted when the open command button is clicked
    void openCommand();

  private:
    void updateButtons();

    QAction* action_open_folder;
    QAction* action_toggle_play;
    QAction* action_stop;
    QAction* action_reset;
    QLabel* label_program_name;
    QLabel* label_program_id;
    QLabel* label_current_file;
    QLabel *label_pid;
    QLabel* label_last_signal;
  };
}// namespace ldb::gui

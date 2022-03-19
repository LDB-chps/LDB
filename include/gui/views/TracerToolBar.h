#pragma once

#include "TracerView.h"
#include <ProcessTracer.h>
#include <QLabel>
#include <QToolBar>

namespace ldb::gui {
  class TracerToolBar : public QToolBar, public TracerView {
    Q_OBJECT
  public:
    TracerToolBar(TracerPanel* parent = nullptr);


  public slots:
    void updateView(SignalEvent evt);
    void startView();

  private slots:
    void updateButtons();
    
  signals:
    // Emitted when the open command button is clicked
    void openCommand();

  private:
    QAction* action_open_folder;
    QAction* action_toggle_play;
    QAction* action_stop;
    QAction* action_reset;
    QAction* action_breakpoints;
    QAction* action_step;
    QLabel* label_program_name;
    QLabel* label_program_id;
    QLabel* label_current_file;
    QLabel* label_pid;
    QLabel* label_last_signal;
  };
}// namespace ldb::gui

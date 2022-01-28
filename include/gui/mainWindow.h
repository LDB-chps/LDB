#pragma once

#include <QMainWindow>
#include "tracerPanel.h"

namespace ldb::gui {

  class MainWindow : public QMainWindow {
  Q_OBJECT

  public:
    explicit MainWindow(QWidget *parent = nullptr);

    // TODO: implement me !
    bool startCommand(const std::string &command, const std::vector<std::string> &args) {
      return tracer_panel->startCommand(command, args);
    }

  private:
    void setupMenuBar();

    void setupTracerPanel();

    TracerPanel *tracer_panel = nullptr;

  };
} // ldb::gui

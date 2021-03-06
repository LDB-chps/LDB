#pragma once
#include <QApplication>
#include <QMainWindow>
#include "TracerPanel.h"

namespace ldb::gui {

  class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    explicit MainWindow(QWidget* parent = nullptr);

    // TODO: implement me !
    bool startCommand(const std::string& command, const std::string& args) {
      return tracer_panel->startExecution(command, args);
    }

    bool startCommand(const std::string& command, const std::vector<std::string>& args) {
      return tracer_panel->startExecution(command, args);
    }

  public slots:
    void startAboutPopup();
    void refreshCSS();

  private:
    void setupMenuBar();

    void setupTracerPanel();

    TracerPanel* tracer_panel = nullptr;
  };
}// namespace ldb::gui

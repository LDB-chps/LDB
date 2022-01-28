#pragma once

#include <QWidget>
#include <QGridLayout>

namespace ldb::gui {

  class TracerPanel : public QWidget {
  Q_OBJECT

  public:
    explicit TracerPanel(QWidget *parent = nullptr);

    bool startCommand(const std::string &command, const std::vector<std::string> &args) {
      return true;
    }

  private:

    void setupToolbar(QGridLayout *layout);

    void setupCodeView();

    void setupVariableView();

    void setupStackedPane();

    /*
    std::unique_ptr<Process> command_process;
    std::unique_ptr<ProcessTracer>
    */
  };
} // ldb::gui

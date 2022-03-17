#pragma once
#include "TracerView.h"
#include "gui/CodeDisplay.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

namespace ldb::gui {


  class CodeView : public QWidget, public TracerView {
  public:
    CodeView(TracerPanel* parent);

  public slots:

    void clearSelection() {
      code_display->setSelectedLine(-1);
    }

    void clearContents() {
      code_display->clear();
      last_path = "";
      clearSelection();
    }

    virtual void refresh() = 0;

  protected:
    std::string last_path;

    CodeDisplay* code_display;
    QString file_path;
    QLabel* label_file_path;
    QVBoxLayout* layout;
  };

}// namespace ldb::gui

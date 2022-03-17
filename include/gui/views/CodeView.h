#pragma once
#include "TracerView.h"
#include "gui/CodeDisplay.h"
#include <QLabel>
#include <QVBoxLayout>

namespace ldb::gui {

  /**
   * @brief The ObjdumpView class is a QWidget that displays the source code of the
   * currently selected process.
   */
  class CodeView : public QWidget, public TracerView {
    Q_OBJECT
  public:
    explicit CodeView(TracerPanel* parent);

  public slots:

    /**
     * @brief Highlight the given line
     * @param line Index of the line to highlight. Set to -1 to disable the highlight
     */
    void setHighlightedLine(int line);

    void updateCodeDisplay();

  private:
    QThread* objdump_thread;
    std::string last_path;
    QString file_path;
    QLabel* label_file_path;
    QVBoxLayout* layout;
    CodeDisplay* code_display;
  };
}// namespace ldb::gui

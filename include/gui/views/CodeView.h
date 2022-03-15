#pragma once

#include "TracerView.h"
#include <ProcessTracer.h>
#include <QLabel>
#include <QPlainTextEdit>
#include <QVBoxLayout>

namespace ldb::gui {

  class CodeDisplay;

  QString getObjdump(const QString& path);

  /**
   * @brief Helper class to display the line numbers in the code view.
   */
  class CodeViewLineWidget : public QWidget {
  public:
    CodeViewLineWidget(CodeDisplay* cv);

    QSize sizeHint() const override;

  protected:
    void paintEvent(QPaintEvent* event) override;

  private:
    CodeDisplay* code_view;
  };

  /**
   * @brief The CodeDisplay class is a QPlainTextEdit dedicated for code display
   */
  class CodeDisplay : public QPlainTextEdit {
    Q_OBJECT
  public:
    explicit CodeDisplay(QWidget* parent = nullptr);


    void lineNumberPaintEvent(QPaintEvent* event);
    int lineNumberWidth();
    void highlightCurrentLine();

  public slots:

    /**
     * @brief Select the line to be highlighted
     * @param line
     */
    void setSelectedLine(int line) {
      selected_line = line;
    };

  private slots:
    void updateLineNumberWidth(int new_block_count);
    void updateLineNumber(const QRect& rect, int dy);

  protected:
    void resizeEvent(QResizeEvent* event) override;

  private:
    CodeViewLineWidget* line_number_widget;

    /**
     * @brief The number of the currently selected line
     * Set to -1 if no line is selected
     */
    int selected_line;
  };

  /**
   * @brief The CodeView class is a QWidget that displays the source code of the
   * currently selected process.
   */
  class CodeView : public QWidget, public TracerView {
    Q_OBJECT
  public:
    explicit CodeView(TracerPanel* parent);

  public slots:
    /**
     * @brief Open the given file in the code view. By default, no line is highlighted.
     * @param path Path of the file to open
     */
    void openFile(const QString& path);

    /**
     * @brief Clear the code view
     */
    void closeFile();

    /**
     * @brief Highlight the given line
     * @param line Index of the line to highlight. Set to -1 to disable the highlight
     */
    void setHighlightedLine(int line);

    void updateCodeDisplay();

  private:
    std::string last_path;
    QString file_path;
    QLabel* label_file_path;
    QVBoxLayout* layout;
    CodeDisplay* code_display;
  };

}// namespace ldb::gui

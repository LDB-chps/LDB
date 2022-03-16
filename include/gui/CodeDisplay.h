#pragma once
#include <QPainter>
#include <QPlainTextEdit>
#include <QTextBlock>

namespace ldb::gui {

  class CodeDisplay;

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

}// namespace ldb::gui
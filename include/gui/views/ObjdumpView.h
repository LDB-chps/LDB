#pragma once

#include "TracerView.h"
#include "gui/CodeDisplay.h"
#include <ProcessTracer.h>
#include <QLabel>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QVBoxLayout>

namespace ldb::gui {

  // Inspired from https://doc.qt.io/qt-5/qtwidgets-richtext-syntaxhighlighter-example.html
  class ObjdumpHighlighter : public QSyntaxHighlighter {
  public:
    ObjdumpHighlighter(QTextDocument* parent = nullptr);

  protected:
    void highlightBlock(const QString& text) override;

  private:
    struct HighlightingRule {
      QRegularExpression pattern;
      QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;
  };

  /**
   * @brief The ObjdumpView class is a QWidget that displays the source code of the
   * currently selected process.
   */
  class ObjdumpView : public QWidget, public TracerView {
    Q_OBJECT
  public:
    explicit ObjdumpView(TracerPanel* parent);

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
    QThread* objdump_thread;
    std::string last_path;
    QString file_path;
    QLabel* label_file_path;
    QVBoxLayout* layout;
    CodeDisplay* code_display;
  };

}// namespace ldb::gui

#include "CodeView.h"
#include "gui/TracerPanel.h"
#include <QFile>
#include <QLayout>
#include <QPainter>
#include <QTextBlock>
#include <array>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

namespace ldb::gui {

  namespace {

    QString getObjdump(const std::filesystem::path& path) {
      std::array<char, 128> buffer = {0};
      QString result;
      std::string cmd = "objdump -d --no-show-raw-insn ";
      cmd += path;

      std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
      if (!pipe) { throw std::runtime_error("popen() failed!"); }
      while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
      }
      return result;
    }

  }// namespace

  CodeViewLineWidget::CodeViewLineWidget(CodeDisplay* cv) : QWidget(cv), code_view(cv) {}

  QSize CodeViewLineWidget::sizeHint() const {
    return QSize(code_view->lineNumberWidth(), 0);
  }

  void CodeViewLineWidget::paintEvent(QPaintEvent* event) {
    code_view->lineNumberPaintEvent(event);
  }

  CodeDisplay::CodeDisplay(QWidget* parent) : QPlainTextEdit(parent), selected_line(-1) {
    setReadOnly(true);
    setWordWrapMode(QTextOption::NoWrap);
    setFont(QFont("monospace", 11));

    line_number_widget = new CodeViewLineWidget(this);

    connect(this, &CodeDisplay::blockCountChanged, this, &CodeDisplay::updateLineNumberWidth);
    connect(this, &CodeDisplay::updateRequest, this, &CodeDisplay::updateLineNumber);
    connect(this, &CodeDisplay::cursorPositionChanged, this, &CodeDisplay::highlightCurrentLine);


    updateLineNumberWidth(0);
    highlightCurrentLine();
  }

  void CodeDisplay::updateLineNumber(const QRect& rect, int dy) {
    if (dy) line_number_widget->scroll(0, dy);
    else
      line_number_widget->update(0, rect.y(), line_number_widget->width(), rect.height());

    if (rect.contains(viewport()->rect())) updateLineNumberWidth(0);
  }

  void CodeDisplay::lineNumberPaintEvent(QPaintEvent* event) {
    QPainter painter(line_number_widget);
    painter.fillRect(event->rect(), "#312F2F");

    QTextBlock block = firstVisibleBlock();
    int block_number = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    if (document()->isEmpty()) return;

    while (block.isValid() && top <= event->rect().bottom()) {
      if (block.isVisible() && bottom >= event->rect().top()) {
        QString number = QString::number(block_number + 1);
        painter.setPen(Qt::gray);
        painter.drawText(0, top, line_number_widget->width(), fontMetrics().height(), Qt::AlignLeft,
                         number);
      }

      block = block.next();
      top = bottom;
      bottom = top + qRound(blockBoundingRect(block).height());
      ++block_number;
    }
  }

  /**
   * @brief Calcule la largeur du widget des nombres de lignes
   *
   * @return int
   */
  int CodeDisplay::lineNumberWidth() {
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
      max /= 10;
      ++digits;
    }

    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * (digits + 3);

    return space;
  }

  void CodeDisplay::resizeEvent(QResizeEvent* event) {
    QPlainTextEdit::resizeEvent(event);

    QRect cr = contentsRect();
    line_number_widget->setGeometry(QRect(cr.left(), cr.top(), lineNumberWidth(), cr.height()));
  }

  void CodeDisplay::updateLineNumberWidth(int new_block_count) {
    setViewportMargins(lineNumberWidth(), 0, 0, 0);
  }

  void CodeDisplay::highlightCurrentLine() {
    if (selected_line <= 0) return;

    QList<QTextEdit::ExtraSelection> extra_selections;

    QTextEdit::ExtraSelection selection;

    QColor line_color = QColor("#312F2F").lighter(160);

    selection.format.setBackground(line_color);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, selected_line);
    selection.cursor = cursor;
    selection.cursor.clearSelection();
    extra_selections.append(selection);
    setExtraSelections(extra_selections);
  }

  CodeView::CodeView(TracerPanel* parent) : QWidget(parent), TracerView(parent) {
    layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    label_file_path = new QLabel(this);
    label_file_path->setText("No file to display");
    layout->addWidget(label_file_path);

    code_display = new CodeDisplay(this);
    code_display->setSelectedLine(-1);
    layout->addWidget(code_display);
    connect(parent, &TracerPanel::tracerUpdated, this, &CodeView::updateCodeDisplay);

    setLayout(layout);
  }

  void CodeView::openFile(const QString& path) {
    QFile file(path);
    if (file.open(QIODevice::ReadOnly)) {
      file_path = path;
      label_file_path->setText(path);
      QString text = file.readAll();
      code_display->setPlainText(text);
      setHighlightedLine(-1);
    }
  }

  void CodeView::closeFile() {
    file_path = "";
    label_file_path->setText("Mo file to display");
    code_display->setPlainText("");
  }

  void CodeView::setHighlightedLine(int line) {
    code_display->setSelectedLine(line);
  }

  void CodeView::updateCodeDisplay() {
    auto tracer = tracer_panel->getTracer();
    if (not tracer) return;
    auto stack = tracer->getStackTrace();
    if (not stack) return;
    auto debug_info = tracer->getDebugInfo();
    auto* symbol_table = debug_info->getSymbolTable();

    if (not symbol_table) return;
    QString object;

    for (auto& it : *stack) {
      auto file = symbol_table.getObjectFileOf(it.getAddress());
      if (file == last_path) break;
      else {
        last_path = file;
        object = getObjdump(file);
        code_display->setPlainText(object);
        break;
      }
    }
  }

}// namespace ldb::gui
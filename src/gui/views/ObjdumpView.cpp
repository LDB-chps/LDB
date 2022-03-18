#include "ObjdumpView.h"
#include "gui/TracerPanel.h"
#include <QFile>
#include <QLayout>
#include <QMessageBox>
#include <QTextBlock>
#include <array>
#include <cstdio>
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


    ObjdumpHighlighter::ObjdumpHighlighter(QTextDocument* parent) : QSyntaxHighlighter(parent) {
      HighlightingRule rule;

      QTextCharFormat format;

      format.setForeground(QColor::fromRgb(125, 220, 125));
      highlightingRules.append(
              HighlightingRule{QRegularExpression(QStringLiteral("<.+>")), format});

      format.setForeground(QColor::fromRgb(242, 90, 90));
      highlightingRules.append(HighlightingRule{
              QRegularExpression(QStringLiteral("<File too big for display>")), format});

      format.setForeground(QColor::fromRgb(97, 242, 194));
      highlightingRules.append(
              HighlightingRule{QRegularExpression(QStringLiteral("[a-f0-9]+:")), format});

      format.setForeground(QColor::fromRgb(145, 242, 145));
      highlightingRules.append(
              HighlightingRule{QRegularExpression(QStringLiteral("[a-f0-9]+ <.+>:")), format});

      format.setForeground(QColor::fromRgb(48, 242, 242));
      highlightingRules.append(
              HighlightingRule{QRegularExpression(QStringLiteral("\\$?0x[a-f0-9]+")), format});

      format.setForeground(QColor::fromRgb(216, 17, 89));
      highlightingRules.append(
              HighlightingRule{QRegularExpression(QStringLiteral("%\\w+")), format});
    }

    void ObjdumpHighlighter::highlightBlock(const QString& text) {
      for (const HighlightingRule& rule : qAsConst(highlightingRules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
          QRegularExpressionMatch match = matchIterator.next();
          setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
      }
    }
  }// namespace

  ObjdumpView::ObjdumpView(TracerPanel* parent) : CodeView(parent) {
    layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    label_file_path = new QLabel(this);
    label_file_path->setText("No file to display");
    label_file_path->setAlignment(Qt::AlignCenter);
    layout->addWidget(label_file_path);

    code_display = new CodeDisplay(this);
    code_display->setSelectedLine(-1);
    layout->addWidget(code_display);
    new ObjdumpHighlighter(code_display->document());

    connect(parent, &TracerPanel::signalReceived, this, &ObjdumpView::refresh);
    connect(parent, &TracerPanel::executionStarted, this, &ObjdumpView::clearContents);
    connect(parent, &TracerPanel::executionEnded, this, &ObjdumpView::clearSelection);
    connect(parent, &TracerPanel::executionEnded, this, [this]() {
      code_display->setSelectedLine(-1);
      last_path.clear();
    });

    setLayout(layout);
  }

  void ObjdumpView::refresh() {
    ProcessTracer* tracer = nullptr;
    std::unique_ptr<StackTrace> stack_trace = nullptr;
    const SymbolTable* symtab = nullptr;

    code_display->setSelectedLine(-1);

    if (not(tracer = tracer_panel->getTracer()) or not tracer->getProcess().isProbeable()) return;

    if (not(stack_trace = tracer->getStackTrace()) or stack_trace->isEmpty()) return;

    if (not tracer->getDebugInfo() or not(symtab = tracer->getDebugInfo()->getSymbolTable()))
      return;


    const auto& frame = *stack_trace->begin();
    auto res = symtab->findInTable(frame.getAddress());

    std::filesystem::path current_object_file;
    // Check if no symbol was returned, or if the symbol is not from the current object file
    if (not res.first or (current_object_file = res.second->getObjectFile()) != last_path) {
      code_display->setPlainText("");
      code_display->setSelectedLine(-1);
      // If we didn't find a symbol, we can't display anything
      if (not res.first) {
        last_path = "";
        return;
      }
    }

    Elf64_Addr current_address = 0;

    // If we are in the same file as previously, no need to re-read the file
    current_address = frame.getAddress() + frame.getOffset() - res.second->getBaseAddress();
    if (current_object_file != last_path) {

      size_t file_size = std::filesystem::file_size(current_object_file);
      if (file_size >= 2 * 1024 * 1024) {
        code_display->setPlainText("<File too big for display>");
        return;
      }

      QString objdump_str = getObjdump(current_object_file);
      // We may fail if we don't have permissions
      if (objdump_str.isEmpty()) return;

      code_display->setPlainText(objdump_str);

      last_path = current_object_file;
    }

    label_file_path->setText(QString::fromStdString(current_object_file));

    // Search the current address in the file
    // Note that we must use the base address, and not the relocated one
    QTextDocument* doc = code_display->document();
    QTextCursor cursor = doc->find(QString::number(current_address, 16) + ":");
    if (cursor.isNull()) return;
    // Qt returns a cursor that has the address selected
    // We clear it
    cursor.clearSelection();
    cursor.movePosition(QTextCursor::StartOfLine);

    // Set the cursor and scroll down to it
    code_display->setTextCursor(cursor);
    code_display->centerCursor();
    code_display->ensureCursorVisible();

    // Highlight the line for show the user where we are
    code_display->setSelectedLine(cursor.blockNumber());
  }

}// namespace ldb::gui
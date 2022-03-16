#include "ObjdumpView.h"
#include "gui/TracerPanel.h"
#include <QFile>
#include <QLayout>
#include <QMessageBox>
#include <QProgressDialog>
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

  }// namespace

  ObjdumpHighlighter::ObjdumpHighlighter(QTextDocument* parent) : QSyntaxHighlighter(parent) {
    HighlightingRule rule;

    QTextCharFormat format;

    format.setForeground(QColor::fromRgb(125, 220, 125));
    highlightingRules.append(HighlightingRule{QRegularExpression(QStringLiteral("<.+>")), format});

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
    highlightingRules.append(HighlightingRule{QRegularExpression(QStringLiteral("%\\w+")), format});
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

  ObjdumpView::ObjdumpView(TracerPanel* parent) : QWidget(parent), TracerView(parent) {
    layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    label_file_path = new QLabel(this);
    label_file_path->setText("No file to display");
    layout->addWidget(label_file_path);

    code_display = new CodeDisplay(this);
    code_display->setSelectedLine(-1);
    layout->addWidget(code_display);
    new ObjdumpHighlighter(code_display->document());

    connect(parent, &TracerPanel::signalReceived, this, &ObjdumpView::updateCodeDisplay);

    setLayout(layout);
  }

  void ObjdumpView::openFile(const QString& path) {
    QFile file(path);
    if (file.open(QIODevice::ReadOnly)) {
      file_path = path;
      label_file_path->setText(path);
      QString text = file.readAll();
      code_display->setPlainText(text);
      setHighlightedLine(-1);
    }
  }

  void ObjdumpView::closeFile() {
    file_path = "";
    label_file_path->setText("Mo file to display");
    code_display->setPlainText("");
  }

  void ObjdumpView::setHighlightedLine(int line) {
    code_display->setSelectedLine(line);
  }

  void ObjdumpView::updateCodeDisplay() {
    ProcessTracer* tracer = nullptr;
    std::unique_ptr<StackTrace> stack_trace = nullptr;
    const SymbolTable* symtab = nullptr;

    if (not(tracer = tracer_panel->getTracer()) or not tracer->getProcess().isProbeable() or
        not(stack_trace = tracer->getStackTrace()) or not tracer->getDebugInfo() or
        not(symtab = tracer->getDebugInfo()->getSymbolTable()))
      return;

    if (stack_trace->isEmpty()) return;

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

      current_address = frame.getAddress() + frame.getOffset() - res.second->getBaseAddress();
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
    setHighlightedLine(cursor.blockNumber());
    code_display->highlightCurrentLine();
  }

}// namespace ldb::gui
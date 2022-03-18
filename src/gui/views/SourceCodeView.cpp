#include "SourceCodeView.h"
#include "gui/TracerPanel.h"
#include <QFile>
#include <QRegularExpression>
#include <QString>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

namespace ldb::gui {

  namespace {
    // Taken from https://github.com/mocmeo/CppSyntaxHighlighter
    // (No time to create an in-depth syntax highlighter for this project)
    //
    // Note that this highlighter is far from complete and shows pretty poor performance on big
    // files But this is not a problem for the current use case. We also assert that the source
    // language is C/C++.
    class CodeViewHighlighter : public QSyntaxHighlighter {
    public:
      CodeViewHighlighter(QTextDocument* parent = nullptr);

    protected:
      void highlightBlock(const QString& text) override;

    private:
      struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
      };
      QVector<HighlightingRule> highlightingRules;

      QRegularExpression commentStartExpression;
      QRegularExpression commentEndExpression;

      QTextCharFormat keywordFormat;
      QTextCharFormat classFormat;
      QTextCharFormat singleLineCommentFormat;
      QTextCharFormat multiLineCommentFormat;
      QTextCharFormat quotationFormat;
      QTextCharFormat functionFormat;
    };


    CodeViewHighlighter::CodeViewHighlighter(QTextDocument* parent) : QSyntaxHighlighter(parent) {
      HighlightingRule rule;

      keywordFormat.setForeground(QColor::fromRgb(216, 17, 89));
      QStringList keywordPatterns;
      // Simple lists of C++ keywords
      // Of course this is not a complete list, but it's enough for our purposes
      keywordPatterns << "\\bchar\\b"
                      << "\\bclass\\b"
                      << "\\bstruct\\b"
                      << "\\bconst\\b"
                      << "\\bdouble\\b"
                      << "\\benum\\b"
                      << "\\bexplicit\\b"
                      << "\\bfriend\\b"
                      << "\\binline\\b"
                      << "\\bint\\b"
                      << "\\blong\\b"
                      << "\\bnamespace\\b"
                      << "\\boperator\\b"
                      << "\\bprivate\\b"
                      << "\\bprotected\\b"
                      << "\\bpublic\\b"
                      << "\\bshort\\b"
                      << "\\bsignals\\b"
                      << "\\bsigned\\b"
                      << "\\bslots\\b"
                      << "\\bstatic\\b"
                      << "\\bstruct\\b"
                      << "\\btemplate\\b"
                      << "\\btypedef\\b"
                      << "\\btypename\\b"
                      << "\\bunion\\b"
                      << "\\bunsigned\\b"
                      << "\\bvirtual\\b"
                      << "\\bvoid\\b"
                      << "\\bvolatile\\b"
                      << "\\binclude\\b"
                      << "\\breturn\\b"
                      << "\\bwhile\\b"
                      << "\\bif\\b"
                      << "\\bfor\\b";
      // Insert each keyword pattern as a rule
      foreach (const QString& pattern, keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
      }

      classFormat.setForeground(QColor::fromRgb(48, 242, 242));
      rule.pattern = QRegularExpression("\\bQ[A-Za-z]+\\b");
      rule.format = classFormat;
      highlightingRules.append(rule);

      quotationFormat.setForeground(QColor::fromRgb(145, 242, 145));
      rule.pattern = QRegularExpression("\".*\"");
      rule.format = quotationFormat;
      highlightingRules.append(rule);


      functionFormat.setForeground(QColor::fromRgb(97, 242, 194));
      rule.pattern = QRegularExpression("\\b[A-Za-z0-9_]+(?=\\()");
      rule.format = functionFormat;
      highlightingRules.append(rule);

      singleLineCommentFormat.setForeground(Qt::gray);
      rule.pattern = QRegularExpression("//[^\n]*");
      rule.format = singleLineCommentFormat;
      highlightingRules.append(rule);

      multiLineCommentFormat.setForeground(Qt::gray);

      commentStartExpression = QRegularExpression("/\\*");
      commentEndExpression = QRegularExpression("\\*/");
    }

    // Taken from https://doc.qt.io/qt-5/qtwidgets-richtext-syntaxhighlighter-example.html
    void CodeViewHighlighter::highlightBlock(const QString& text) {
      for (const HighlightingRule& rule : qAsConst(highlightingRules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
          QRegularExpressionMatch match = matchIterator.next();
          setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
      }

      setCurrentBlockState(0);


      int startIndex = 0;
      if (previousBlockState() != 1) startIndex = text.indexOf(commentStartExpression);

      while (startIndex >= 0) {
        QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
        int endIndex = match.capturedStart();
        int commentLength = 0;
        if (endIndex == -1) {
          setCurrentBlockState(1);
          commentLength = text.length() - startIndex;
        } else {
          commentLength = endIndex - startIndex + match.capturedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
      }
    }
  }// namespace


  SourceCodeView::SourceCodeView(TracerPanel* parent) : CodeView(parent) {
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
    new CodeViewHighlighter(code_display->document());

    connect(parent, &TracerPanel::signalReceived, this, &SourceCodeView::refresh);
    connect(parent, &TracerPanel::executionEnded, this, &SourceCodeView::clearSelection);
    connect(parent, &TracerPanel::executionStarted, this, &SourceCodeView::clearContents);

    setLayout(layout);
  }

  void SourceCodeView::refresh() {
    ProcessTracer* tracer = nullptr;
    std::unique_ptr<StackTrace> stack_trace = nullptr;
    const SymbolTable* symtab = nullptr;

    // Unselect the current line
    // If we fail to update the view, we don't want the previous line to be displayed
    code_display->setSelectedLine(-1);


    // TODO: Refactor me !
    if (not(tracer = tracer_panel->getTracer()) or not tracer->getProcess().isProbeable() or
        not(stack_trace = tracer->getStackTrace()) or not tracer->getDebugInfo() or
        not(symtab = tracer->getDebugInfo()->getSymbolTable()))
      return;

    if (stack_trace->isEmpty()) return;

    std::filesystem::path source_file;
    int current_line = 0;

    // Iterate on the stack until we find a function we know the source file of
    for (auto& it : *stack_trace) {
      auto res = symtab->findInTable(it.getAddress());
      if (not res.first) continue;
      std::filesystem::path current_object_file;
      size_t addr = it.getAddress() + it.getOffset() - res.second->getBaseAddress();

      // If the process is paused, rip points to the next instruction
      // Meaning we will get the next line, and not the current one
      // To prevent this, we subtract 1 from the address before passing it to addr2line
      //
      // Note that this is not true for a process that has crashed, because rip points to the
      // instruction that caused the crash
      if (tracer->getProcess().getStatus() == Process::Status::kStopped) addr -= 1;

      auto tmp = addr2Line(res.second->getObjectFile(), addr);
      if (tmp and not tmp->first.empty()) {
        source_file = tmp->first;
        current_line = tmp->second;
        break;
      }
    }

    // If we failed to find a source file, just return
    if (source_file.empty()) {
      last_path = "";
      return;
    }

    // If we are in the same file as previously, no need to re-read the file, we can just update the
    // line
    if (source_file != last_path) {

      QFile file(QString::fromStdString(source_file));
      if (not file.open(QIODevice::ReadOnly)) {
        code_display->setPlainText("<Unable to open file>");
        last_path = "";
        return;
      }
      label_file_path->setText(QString::fromStdString(source_file));
      QString source_code = file.readAll();
      code_display->setPlainText(source_code);

      last_path = source_file;
    }

    // Qt TextEdit starts at line 0, but we start at line 1
    // So we must subtract 1 from the line number
    code_display->setSelectedLine(current_line - 1);

    // Focus the view on the current line
    QTextCursor cursor = code_display->textCursor();
    cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, current_line - 1);
    code_display->setTextCursor(cursor);
    code_display->centerCursor();
  }


}// namespace ldb::gui
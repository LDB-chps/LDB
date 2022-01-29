#include "codeView.h"


namespace ldb::gui {

  CodeView::CodeView(QWidget* parent) : QTextEdit(parent) {
    setReadOnly(true);
    setLineWrapMode(QTextEdit::NoWrap);
    setWordWrapMode(QTextOption::NoWrap);
    setFont(QFont("Courier", 10));
  }
}// namespace ldb::gui
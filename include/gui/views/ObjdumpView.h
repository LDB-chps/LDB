#pragma once

#include "CodeView.h"
#include <ProcessTracer.h>
#include <QLabel>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QVBoxLayout>

namespace ldb::gui {

  /**
   * @brief The ObjdumpView class is a QWidget that displays the source code of the
   * currently selected process.
   */
  class ObjdumpView : public CodeView {
    Q_OBJECT
  public:
    explicit ObjdumpView(TracerPanel* parent);

  public slots:

    void refresh() override;

  private:
    std::string last_path;
  };

}// namespace ldb::gui

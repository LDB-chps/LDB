#pragma once
#include "CodeView.h"
#include <QLabel>
#include <QVBoxLayout>

namespace ldb::gui {

  /**
   * @brief The SourceCodeView class is a QWidget that displays the source code of the
   * currently selected process.
   */
  class SourceCodeView : public CodeView {
    Q_OBJECT
  public:
    explicit SourceCodeView(TracerPanel* parent);

  public slots:

    void refresh() override;

  private:
  };
}// namespace ldb::gui

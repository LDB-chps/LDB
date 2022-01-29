#pragma once

#include <QTextEdit>
#include <processTracer.h>

namespace ldb::gui {

  class CodeView : public QTextEdit {
    Q_OBJECT
  public:
    explicit CodeView(QWidget* parent = nullptr);

    /**
     * @brief Update the view to reflect the tracer state
     * Display the current file and line number, or a message if none of this data is available
     * @param tracer A pointer to the tracer. If nullptr, the view should present a blank state.
     */
    void update(ProcessTracer* tracer) {}

  private:
  };

}// namespace ldb::gui

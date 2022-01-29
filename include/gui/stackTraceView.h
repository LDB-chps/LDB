#pragma once

#include <QTreeView>
#include <processTracer.h>

namespace ldb::gui {

  class StackTraceView : public QTreeView {
    Q_OBJECT
  public:
    explicit StackTraceView(QWidget* parent = nullptr);

    /**
     * @brief Update the view to reflect the tracer state
     * @param tracer A pointer to the tracer. If nullptr, the view should present a blank state.
     */
    void update(ProcessTracer* tracer) {}

  private:
  };
}// namespace ldb::gui

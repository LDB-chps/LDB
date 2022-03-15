#pragma once

#include "TracerView.h"
#include <ProcessTracer.h>
#include <QTreeWidget>

namespace ldb::gui {

  class StackTraceView : public QTreeWidget, public TracerView {
    Q_OBJECT
  public:
    explicit StackTraceView(TracerPanel* parent);

  public slots:

    /**
     * @brief Update the view to reflect the tracer state
     * @param tracer A pointer to the tracer. If nullptr, the view should present a blank state.
     */
    void updateView();

  private:
  };
}// namespace ldb::gui

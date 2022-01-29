#pragma once

#include <QToolBar>
#include <processTracer.h>

namespace ldb::gui {
  class TracerToolBar : public QToolBar {
    Q_OBJECT
  public:
    TracerToolBar(QWidget* parent = nullptr);

    /**
     * @brief Update the view to reflect the tracer state
     * @param tracer A pointer to the tracer. If nullptr, the view should present a blank state.
     */
    void update(ProcessTracer* tracer) {}

  private:
  };
}// namespace ldb::gui

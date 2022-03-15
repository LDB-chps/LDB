#pragma once

#include "TracerView.h"
#include <ProcessTracer.h>
#include <QFrame>
#include <QTabWidget>
#include <QTableWidget>

namespace ldb::gui {

  class VariableView : public QFrame, public TracerView {
    Q_OBJECT

  public:
    explicit VariableView(TracerPanel* parent);

  public slots:

    /**
     * @brief Update the view to reflect the tracer state
     * @param tracer A pointer to the tracer. If nullptr, the view should present a blank state.
     */
    void updateView();

  private:
    void fillTableFromSnapshot(const RegistersSnapshot& snapshot);

    QTabWidget* tabs;
    QTableWidget* variables;
    QTableWidget* registers;
  };
}// namespace ldb::gui

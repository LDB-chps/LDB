#pragma once

#include "tracerView.h"
#include <QFrame>
#include <QTabWidget>
#include <QTableView>
#include <processTracer.h>

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
    void update();

  private:
    void fillTableFromSnapshot(const RegistersSnapshot& snapshot);

    QTabWidget* tabs;
    QTableView* variables;
    QTableView* registers;
  };
}// namespace ldb::gui

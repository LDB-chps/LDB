//
// Created by thukisdo on 28/01/2022.
//

#pragma once

#include <QFrame>
#include <processTracer.h>

namespace ldb::gui {

  class VariableView : public QFrame {
    Q_OBJECT

  public:
    explicit VariableView(QWidget* parent = nullptr);

    /**
     * @brief Update the view to reflect the tracer state
     * @param tracer A pointer to the tracer. If nullptr, the view should present a blank state.
     */
    void update(ProcessTracer* tracer) {}

  private:
  };
}// namespace ldb::gui

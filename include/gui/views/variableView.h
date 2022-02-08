//
// Created by thukisdo on 28/01/2022.
//

#pragma once

#include <QFrame>
#include <QTabWidget>
#include <QTableView>
#include <processTracer.h>

namespace ldb::gui {

  class VariableView : public QFrame {
    Q_OBJECT

  public:
    explicit VariableView(QWidget* parent = nullptr);

  public slots:

    /**
     * @brief Update the view to reflect the tracer state
     * @param tracer A pointer to the tracer. If nullptr, the view should present a blank state.
     */
    void update() {}

  private:
    QTabWidget* tabs;
    QTableView* variables;
    QTableView* registers;
  };
}// namespace ldb::gui

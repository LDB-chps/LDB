//
// Created by thukisdo on 28/01/2022.
//

// You may need to build the project (run Qt uic code generator) to get "ui_variableView.h" resolved

#include "variableView.h"
#include <QGridLayout>
#include <QLabel>

namespace ldb::gui {
  VariableView::VariableView(QWidget* parent) : QFrame(parent) {
    tabs = new QTabWidget(this);
    variables = new QTableView;
    variables->setSelectionBehavior(QAbstractItemView::SelectRows);
    tabs->addTab(variables, "Variables");

    registers = new QTableView;
    registers->setSelectionBehavior(QAbstractItemView::SelectRows);
    tabs->addTab(registers, "Registers");
  }

}// namespace ldb::gui

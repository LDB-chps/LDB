#include "variableView.h"
#include "gui/tracerPanel.h"
#include <QGridLayout>
#include <QLabel>
#include <QStandardItemModel>

namespace ldb::gui {

  VariableView::VariableView(TracerPanel* parent) : QFrame(parent), TracerView(parent) {
    tabs = new QTabWidget(this);
    variables = new QTableView;
    variables->setSelectionBehavior(QAbstractItemView::SelectRows);
    variables->setEditTriggers(QAbstractItemView::NoEditTriggers);
    variables->setModel(new QStandardItemModel);
    tabs->addTab(variables, "Variables");

    registers = new QTableView;
    registers->setSelectionBehavior(QAbstractItemView::SelectRows);
    registers->setEditTriggers(QAbstractItemView::NoEditTriggers);
    registers->setModel(new QStandardItemModel);

    tabs->addTab(registers, "Registers");
    connect(parent, &TracerPanel::tracerUpdated, this, &VariableView::update);
    connect(parent, &TracerPanel::executionStarted, this, &VariableView::update);
  }


  void VariableView::update() {

    auto tracer = tracer_panel->getTracer();
    if (tracer == nullptr) { return; }
    auto snapshot = tracer->getRegistersSnapshot();
    if (snapshot == nullptr) { return; }

    auto* model = registers->model();

    // Empty the table
    for (int i = 0; i < model->rowCount(); i++) { model->removeRow(0); }

    // Add the registers
    for (auto& reg : *snapshot) {
      model->insertRow(0);
      model->setData(model->index(0, 0), QString::fromStdString(reg.getName()));
      model->setData(model->index(0, 1), QString::fromStdString(reg.getValueAsString()));
    }

    registers->resizeColumnsToContents();
    registers->resizeRowsToContents();
  }

  void VariableView::fillTableFromSnapshot(const RegistersSnapshot& snapshot) {}

}// namespace ldb::gui

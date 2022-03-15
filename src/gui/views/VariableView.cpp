#include "VariableView.h"
#include "gui/TracerPanel.h"
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QStandardItemModel>
namespace ldb::gui {

  VariableView::VariableView(TracerPanel* parent) : TracerView(parent) {

    QGridLayout* layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 5, 0);
    setLayout(layout);

    registers = new QTableWidget();
    registers->setColumnCount(2);
    registers->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    registers->verticalHeader()->setVisible(false);
    registers->horizontalHeader()->setVisible(true);
    registers->setSelectionBehavior(QAbstractItemView::SelectRows);
    registers->setEditTriggers(QAbstractItemView::NoEditTriggers);

    registers->setAlternatingRowColors(true);
    registers->setSelectionBehavior(QAbstractItemView::SelectRows);
    registers->setSelectionMode(QAbstractItemView::NoSelection);
    registers->setWordWrap(true);
    registers->setContextMenuPolicy(Qt::CustomContextMenu);

    QStringList headers;
    headers << "Register"
            << "Value";
    registers->setHorizontalHeaderLabels(headers);
    registers->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter |
                                                       (Qt::Alignment) Qt::TextWordWrap);
    registers->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    updateView();

    layout->addWidget(registers, 0, 0);
    connect(parent, &TracerPanel::tracerUpdated, this, &VariableView::updateView);
  }


  void VariableView::updateView() {

    auto tracer = tracer_panel->getTracer();
    if (tracer == nullptr) { return; }

    auto snapshot = tracer->getRegistersSnapshot();

    if (snapshot) {
      // Empty the table
      registers->clearContents();
      registers->setRowCount(0);

      int i = 0;
      for (auto& reg : *snapshot) {
        registers->insertRow(i);
        registers->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(reg.getName())));
        registers->setItem(i, 1,
                           new QTableWidgetItem(QString::fromStdString(reg.getValueAsString())));
        i++;
      }
    }

    registers->setWordWrap(true);
    // registers->resizeColumnsToContents();
  }

  void VariableView::fillTableFromSnapshot(const RegistersSnapshot& snapshot) {}

}// namespace ldb::gui

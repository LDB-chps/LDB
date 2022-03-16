#include "LibraryView.h"
#include <QHeaderView>
#include <gui/TracerPanel.h>

namespace ldb::gui {

  LibraryView::LibraryView(TracerPanel* parent) : QTableWidget(parent), TracerView(parent) {
    setColumnCount(1);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    verticalHeader()->setVisible(false);
    horizontalHeader()->setVisible(true);
    horizontalHeader()->setStretchLastSection(true);

    QStringList headers;
    headers << "Shared libraries";
    setHorizontalHeaderLabels(headers);

    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setAlternatingRowColors(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::NoSelection);
    setWordWrap(true);
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(parent, &TracerPanel::executionStarted, this, &LibraryView::onExecutionStarted);
    connect(parent, &TracerPanel::signalReceived, this, &LibraryView::onExecutionStarted);
    connect(parent, &TracerPanel::executionEnded, this, &LibraryView::clearContents);
  }

  void LibraryView::onExecutionStarted() {
    clearContents();
    auto* tracer = tracer_panel->getTracer();

    if (not tracer) return;

    const auto* debug_info = tracer->getDebugInfo();

    if (not debug_info) return;

    const auto& libraries = debug_info->getSharedLibraries();
    setRowCount(libraries.size());

    int i = 0;
    for (const auto& library : libraries) {
      setItem(i, 0, new QTableWidgetItem(QString::fromStdString(library)));
      i++;
    }
  }

}// namespace ldb::gui
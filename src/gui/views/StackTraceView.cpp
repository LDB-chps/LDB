#include "StackTraceView.h"
#include "gui/TracerPanel.h"
#include <QHeaderView>

namespace ldb::gui {
  StackTraceView::StackTraceView(TracerPanel* parent) : QTreeWidget(parent), TracerView(parent) {
    setRootIsDecorated(false);
    setAlternatingRowColors(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setUniformRowHeights(true);
    setWordWrap(true);
    setIndentation(0);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setColumnCount(3);

    connect(parent, &TracerPanel::tracerUpdated, this, &StackTraceView::updateView);
    connect(parent, &TracerPanel::executionStarted, this, &QTreeWidget::clear);
    connect(parent, &TracerPanel::executionEnded, this, &QTreeWidget::clear);

    QStringList headerLabels;
    headerLabels << "Adress"
                 << "Offset"
                 << "Function";
    setHeaderLabels(headerLabels);
    header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
  }


  void StackTraceView::updateView() {
    auto tracer = tracer_panel->getTracer();
    if (not tracer) return;

    this->clear();

    auto stacktrace = tracer->getStackTrace();
    if (not stacktrace) return;

    for (const auto& frame : *stacktrace) {
      auto item = new QTreeWidgetItem(this);
      item->setText(0, QString::number(frame.getAddress(), 16));
      item->setText(1, QString::number(frame.getOffset(), 16));
      item->setText(2, QString::fromStdString(frame.getFunctionName()));
    }
  }
}// namespace ldb::gui

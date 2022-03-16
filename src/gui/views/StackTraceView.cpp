#include "StackTraceView.h"
#include "gui/TracerPanel.h"
#include <QHeaderView>

namespace ldb::gui {
  StackTraceView::StackTraceView(TracerPanel* parent) : QTreeWidget(parent), TracerView(parent) {
    setRootIsDecorated(true);

    setSelectionMode(QAbstractItemView::NoSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setUniformRowHeights(true);
    // setAlternatingRowColors(true);
    setWordWrap(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setColumnCount(4);

    connect(parent, &TracerPanel::signalReceived, this, &StackTraceView::updateView);
    connect(parent, &TracerPanel::executionStarted, this, &QTreeWidget::clear);
    connect(parent, &TracerPanel::executionEnded, this, &QTreeWidget::clear);

    QStringList headerLabels;
    headerLabels << "Adress"
                 << "Offset"
                 << "Function"
                 << "File";
    setHeaderLabels(headerLabels);
    header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(3, QHeaderView::Stretch);
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

      if (not frame.getSymbol()) continue;
      const Symbol* sym = frame.getSymbol();

      const std::string& file = sym->getFile();
      item->setText(3, QString::fromStdString(sym->getFile()));

      QBrush child_color = QBrush(QColor::fromRgb(35, 35, 35));
      for (auto& var : sym->getArgs()) {
        auto* child = new QTreeWidgetItem(item);
        child->setText(0, "Parameter");
        child->setText(1, QString::fromStdString(var.getType()));
        child->setText(2, QString::fromStdString(var.getName()));
        child->setBackground(0, child_color);
        child->setBackground(1, child_color);
        child->setBackground(2, child_color);
        child->setBackground(3, child_color);
        item->addChild(child);
      }

      for (auto& var : sym->getVars()) {
        auto* child = new QTreeWidgetItem(item);
        child->setText(0, "Variable");
        child->setText(1, QString::fromStdString(var.getType()));
        child->setText(2, QString::fromStdString(var.getName()));
        child->setBackground(0, child_color);
        child->setBackground(1, child_color);
        child->setBackground(2, child_color);
        child->setBackground(3, child_color);
        item->addChild(child);
      }
    }

    auto* item = itemAt(0, 0);
    if (not item) return;
    QBrush top_item = QBrush(QColor::fromRgb(80, 55, 55));
    item->setBackground(0, top_item);
    item->setBackground(1, top_item);
    item->setBackground(2, top_item);
    item->setBackground(3, top_item);
  }
}// namespace ldb::gui

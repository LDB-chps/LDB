#include "stackTraceView.h"


namespace ldb::gui {
  StackTraceView::StackTraceView(QWidget* parent) : QTreeView(parent) {
    setRootIsDecorated(false);
    setAlternatingRowColors(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSortingEnabled(true);
    setUniformRowHeights(true);
    setAllColumnsShowFocus(true);
    setWordWrap(true);
    setTextElideMode(Qt::ElideMiddle);
    setIndentation(0);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setContextMenuPolicy(Qt::CustomContextMenu);
  }
}// namespace ldb::gui

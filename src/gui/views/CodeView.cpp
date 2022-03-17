#include "CodeView.h"
#include "gui/TracerPanel.h"

namespace ldb::gui {
  CodeView::CodeView(TracerPanel* parent) : QWidget(parent), TracerView(parent) {}
}// namespace ldb::gui
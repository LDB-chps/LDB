#pragma once
#include "TracerView.h"
#include <QTableWidget>

namespace ldb::gui {

  class LibraryView : public QTableWidget, public TracerView {
  public:
    LibraryView(TracerPanel* parent);

  public slots:

    void onExecutionStarted();
  };

};// namespace ldb::gui

#pragma once
#include "TracerView.h"
#include <QDialog>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTableView>

namespace ldb::gui {

  class BreakpointsDialog : public QDialog, public TracerView {
  public:
    BreakpointsDialog(TracerPanel* parent);

  public slots:

    void makeModel();
    void clearModel();

  private:
    QLineEdit* search_bar;
    QCompleter* completer;
    QStandardItemModel* model;
    QSortFilterProxyModel* search_proxy;
    QTableView* bp_list_view;
  };

}// namespace ldb::gui

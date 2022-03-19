#pragma once
#include "SymbolTable.h"
#include "TracerView.h"
#include <QDialog>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QTableView>

namespace ldb::gui {

  class BreakpointModel : public QAbstractTableModel {
  public:
    BreakpointModel(QObject* parent, TracerPanel* tp);
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    const Symbol* getSymbol(int row) const;

    bool isBreakpoint(const Symbol* sym) const;
    void toggleBreakpoint(const QModelIndex& index);

  public slots:
    void update();

  private:
    TracerPanel* tracer_panel;
  };

  class BreakpointsDialog : public QDialog, public TracerView {
  public:
    explicit BreakpointsDialog(TracerPanel* parent);

  public slots:

    void makeModel();
    void clearModel();

    virtual QSize sizeHint() const override {
      return QSize(800, 600);
    }


  private:
    QLineEdit* search_bar;
    BreakpointModel* model;
    QSortFilterProxyModel* search_proxy;
    QSortFilterProxyModel* breakpoint_proxy;
    QTableView* function_list;
    QTableView* breakpoint_list;
  };

}// namespace ldb::gui

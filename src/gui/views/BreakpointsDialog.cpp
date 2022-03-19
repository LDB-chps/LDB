#include "BreakpointsDialog.h"
#include "gui/TracerPanel.h"
#include <QHeaderView>
#include <QVBoxLayout>

namespace ldb::gui {

  namespace {
    // We define our own proxy model to be able to sort by breakpoints status
    //
    // Since QModels must operate on any model, if we're not dealing with a BreakpointModel model,
    // all calls are redirected to the parent class
    // Rows are sorted using the QSortFilterProxyModel::lessThan() method
    class BreakpointSortProxyModel : public QSortFilterProxyModel {
    public:
      explicit BreakpointSortProxyModel(BreakpointModel* parent, bool sbreak = true)
          : QSortFilterProxyModel(parent), source_model(parent), show_breakpoints(sbreak) {
        QSortFilterProxyModel::setSourceModel(parent);
      }

      int rowCount(const QModelIndex& parent) const {
        return source_model->rowCount();
      }

      int columnCount(const QModelIndex& parent) const {
        return source_model->columnCount();
      }

    protected:
      // We must redefine the following methods to be able to sort by breakpoints status
      bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
      bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

      // We override this function to set the source_model to nullptr if it's not a BreakpointModel
      void setSourceModel(QAbstractItemModel* sourceModel) override;
      void setSourceModel(BreakpointModel* source_model);

    private:
      BreakpointModel* source_model;
      bool show_breakpoints = true;
    };

    bool BreakpointSortProxyModel::filterAcceptsRow(int sourceRow,
                                                    const QModelIndex& sourceParent) const {
      if (not source_model) return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
      auto* sym = source_model->getSymbol(sourceRow);
      if (sym == nullptr) { return false; }
      return ((show_breakpoints and source_model->isBreakpoint(sym)) or
              (not show_breakpoints and not source_model->isBreakpoint(sym))) and
             QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
    }

    bool BreakpointSortProxyModel::lessThan(const QModelIndex& left,
                                            const QModelIndex& right) const {
      if (not source_model) return QSortFilterProxyModel::lessThan(left, right);

      auto left_data = source_model->getSymbol(left.row());
      auto right_data = source_model->getSymbol(left.row());

      if (not left_data) {
        return false;
      } else if (not right_data) {
        return true;
      } else
        return QSortFilterProxyModel::lessThan(left, right);
    }

    void BreakpointSortProxyModel::setSourceModel(BreakpointModel* source_model) {
      QSortFilterProxyModel::setSourceModel(source_model);
      this->source_model = source_model;
    }

    void BreakpointSortProxyModel::setSourceModel(QAbstractItemModel* sourceModel) {
      QSortFilterProxyModel::setSourceModel(source_model);
      this->source_model = dynamic_cast<BreakpointModel*>(sourceModel);
    }
  }// namespace

  BreakpointModel::BreakpointModel(QObject* parent, TracerPanel* tp)
      : QAbstractTableModel(parent), tracer_panel(tp) {}

  int BreakpointModel::rowCount(const QModelIndex& parent) const {
    auto* tracer = tracer_panel->getTracer();

    const SymbolTable* symtab = nullptr;
    if (not tracer or not(symtab = tracer->getSymbolTable())) return 0;
    return symtab->getSize();
  }

  int BreakpointModel::columnCount(const QModelIndex& parent) const {
    return 3;
  }

  void BreakpointModel::update() {
    beginResetModel();
    endResetModel();
  }

  QVariant BreakpointModel::data(const QModelIndex& index, int role) const {
    if (not index.isValid()) return {};
    if (role == Qt::DisplayRole) {
      auto* tracer = tracer_panel->getTracer();
      const SymbolTable* symtab = nullptr;
      if (not tracer or not(symtab = tracer->getSymbolTable())) return {};

      auto* symbol = symtab->at(index.row());
      if (not symbol) return {};

      switch (index.column()) {
        case 0:
          return QVariant::fromValue("0x" + QString::number(symbol->getAddress(), 16));
        case 1:
          return QVariant::fromValue(QString::fromStdString(symbol->getName()));
        case 2:
          return QVariant::fromValue(QString::fromStdString(symbol->getFile().string()));
        default:
          return {};
      }
    }
    return {};
  }

  QVariant BreakpointModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole) {
      if (orientation == Qt::Horizontal) {
        switch (section) {
          case 0:
            return "Address";
          case 1:
            return "Name";
          case 2:
            return "File";
          default:
            return {};
        }
      }
    }
    return {};
  }

  const Symbol* BreakpointModel::getSymbol(int row) const {
    auto* tracer = tracer_panel->getTracer();
    if (not tracer) return nullptr;
    auto* debug_info = tracer->getDebugInfo();
    if (not debug_info) return nullptr;
    auto* symtab = debug_info->getSymbolTable();
    if (not symtab) return nullptr;

    return symtab->at(row);
  }

  bool BreakpointModel::isBreakpoint(const Symbol* sym) const {
    auto* tracer = tracer_panel->getTracer();
    if (not tracer) return false;
    auto* breakpoints = tracer->getBreakPointHandler();
    if (not breakpoints) return false;
    return breakpoints->isBreakPoint(sym->getAddress());
  }

  void BreakpointModel::toggleBreakpoint(const QModelIndex& pos) {
    auto* tracer = tracer_panel->getTracer();
    if (not tracer) return;
    auto* breakpoints = tracer->getBreakPointHandler();
    if (not breakpoints) return;
    auto* debug_info = tracer->getDebugInfo();
    if (not debug_info) return;
    auto* symtab = debug_info->getSymbolTable();
    if (not symtab) return;

    auto* symbol = symtab->at(pos.row());
    if (not symbol) return;

    if (isBreakpoint(symbol)) breakpoints->remove(*symbol);
    else
      breakpoints->add(*symbol);
    emit dataChanged(pos, pos);
  }

  BreakpointsDialog::BreakpointsDialog(TracerPanel* parent) : TracerView(parent), model(nullptr) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    setWindowTitle("Breakpoints");

    QTabWidget* tab_widget = new QTabWidget(this);
    tab_widget->tabBar()->setExpanding(true);
    tab_widget->tabBar()->setDocumentMode(true);
    layout->addWidget(tab_widget);

    QWidget* search_panel = new QWidget(this);
    QVBoxLayout* search_layout = new QVBoxLayout(search_panel);
    search_layout->setContentsMargins(0, 0, 0, 0);
    search_layout->setSpacing(0);
    search_panel->setLayout(search_layout);
    tab_widget->addTab(search_panel, "Add breakpoints");

    search_bar = new QLineEdit(this);
    search_bar->setPlaceholderText("Search symbol");
    search_layout->addWidget(search_bar);

    function_list = new QTableView(this);
    function_list->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    function_list->verticalHeader()->setVisible(false);
    function_list->horizontalHeader()->setVisible(true);
    function_list->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    function_list->setSelectionBehavior(QAbstractItemView::SelectRows);
    function_list->setEditTriggers(QAbstractItemView::NoEditTriggers);
    function_list->setWordWrap(true);
    search_layout->addWidget(function_list);

    QWidget* active_breakpoint_panel = new QWidget(this);
    QVBoxLayout* active_layout = new QVBoxLayout(active_breakpoint_panel);
    active_layout->setContentsMargins(0, 0, 0, 0);
    active_breakpoint_panel->setLayout(active_layout);
    tab_widget->addTab(active_breakpoint_panel, "Active breakpoints");

    breakpoint_list = new QTableView(this);
    breakpoint_list->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    breakpoint_list->verticalHeader()->setVisible(false);
    breakpoint_list->horizontalHeader()->setVisible(true);
    breakpoint_list->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    breakpoint_list->setSelectionBehavior(QAbstractItemView::SelectRows);
    breakpoint_list->setEditTriggers(QAbstractItemView::NoEditTriggers);
    breakpoint_list->setWordWrap(true);
    active_layout->addWidget(breakpoint_list);

    connect(parent, &TracerPanel::executionStarted, this, &BreakpointsDialog::makeModel);
    connect(parent, &TracerPanel::executionEnded, this, &BreakpointsDialog::clearModel);
    clearModel();
  }

  void BreakpointsDialog::makeModel() {
    clearModel();
    model->update();
  }

  void BreakpointsDialog::clearModel() {
    delete model;
    model = new BreakpointModel(this, tracer_panel);
    search_proxy = new BreakpointSortProxyModel(model, false);
    search_proxy->setFilterKeyColumn(1);
    search_proxy->setFilterFixedString(search_bar->text());
    function_list->setModel(search_proxy);
    connect(function_list, &QTableView::doubleClicked, model, [&](const QModelIndex& index) {
      model->toggleBreakpoint(search_proxy->mapToSource(index));
    });
    connect(search_bar, &QLineEdit::textChanged, search_proxy,
            &QSortFilterProxyModel::setFilterFixedString);

    breakpoint_proxy = new BreakpointSortProxyModel(model, true);
    breakpoint_proxy->setFilterKeyColumn(1);
    breakpoint_list->setModel(breakpoint_proxy);
    connect(breakpoint_list, &QTableView::doubleClicked, model, [&](const QModelIndex& index) {
      model->toggleBreakpoint(breakpoint_proxy->mapToSource(index));
    });
  }

}// namespace ldb::gui

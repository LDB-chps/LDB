#include "BreakpointsDialog.h"
#include "gui/TracerPanel.h"
#include <QHeaderView>
#include <QVBoxLayout>

namespace ldb::gui {

  namespace {
    class BreakpointSortProxyModel : public QSortFilterProxyModel {
    public:
      BreakpointSortProxyModel(BreakpointModel* parent = nullptr)
          : QSortFilterProxyModel(parent), source_model(parent) {}

    protected:
      bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override {
        auto* sym = source_model->getSymbol(sourceRow);
        if (sym == nullptr) { return false; }
        return false;
      }

      bool lessThan(const QModelIndex& left, const QModelIndex& right) const override {
        auto left_data = source_model->getSymbol(left.row());
        auto right_data = source_model->getSymbol(left.row());

        if (not left_data) {
          return false;
        } else if (not right_data) {
          return true;
        } else {
          return left_data->getName() < right_data->getName();
        }
      }

      void setSourceModel(BreakpointModel* source_model) {
        QSortFilterProxyModel::setSourceModel(source_model);
        this->source_model = source_model;
      }

    private:
      BreakpointModel* source_model;
    };
  }// namespace

  BreakpointModel::BreakpointModel(QObject* parent, TracerPanel* tp)
      : QAbstractTableModel(parent), tracer_panel(tp) {}

  int BreakpointModel::rowCount(const QModelIndex& parent) const {
    auto* tracer = tracer_panel->getTracer();
    if (not tracer) return 0;
    auto* debug_info = tracer->getDebugInfo();
    if (not debug_info) return 0;
    auto* symtab = debug_info->getSymbolTable();
    if (not symtab) return 0;
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
      if (not tracer) return {};
      auto* debug_info = tracer->getDebugInfo();
      if (not debug_info) return {};
      auto* symtab = debug_info->getSymbolTable();
      if (not symtab) return {};

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

  BreakpointsDialog::BreakpointsDialog(TracerPanel* parent) : TracerView(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    setWindowTitle("Breakpoints");

    model = new BreakpointModel(this, parent);
    search_proxy = new QSortFilterProxyModel(this);
    search_proxy->setSourceModel(model);
    search_proxy->setFilterKeyColumn(1);
    // search_proxy->sort(2, Qt::DescendingOrder);


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


    connect(search_bar, &QLineEdit::textChanged, search_proxy,
            &QSortFilterProxyModel::setFilterFixedString);

    function_list = new QTableView(this);
    function_list->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    function_list->verticalHeader()->setVisible(false);
    function_list->horizontalHeader()->setVisible(true);
    function_list->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    function_list->setSelectionBehavior(QAbstractItemView::SelectRows);
    function_list->setEditTriggers(QAbstractItemView::NoEditTriggers);
    function_list->setModel(search_proxy);
    function_list->setWordWrap(true);
    search_layout->addWidget(function_list);

    breakpoint_proxy = new BreakpointSortProxyModel(model);
    breakpoint_proxy->setSourceModel(model);

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
    breakpoint_list->setModel(breakpoint_proxy);
    breakpoint_list->setWordWrap(true);
    active_layout->addWidget(breakpoint_list);

    connect(parent, &TracerPanel::signalReceived, this, &BreakpointsDialog::makeModel);
    connect(parent, &TracerPanel::executionEnded, this, &BreakpointsDialog::clearModel);
  }

  void BreakpointsDialog::makeModel() {
    model->update();
  }

  void BreakpointsDialog::clearModel() {
    delete model;
    model = new BreakpointModel(this, tracer_panel);
    search_proxy->setSourceModel(model);
    function_list->setModel(search_proxy);
  }

}// namespace ldb::gui

#include "tracerPanel.h"
#include "commandDialog.h"
#include "logWidget.h"
#include <QLabel>
#include <QProgressBar>
#include <QSplitter>
#include <QTabWidget>
#include <QTextEdit>
#include <QToolBar>
#include <QToolButton>
#include <tscl.hpp>

namespace ldb::gui {
  TracerPanel::TracerPanel(QWidget* parent) : QWidget(parent) {

    auto* layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setupToolbar(layout);

    auto* splitter = new QSplitter(Qt::Vertical, this);
    layout->addWidget(splitter, 1, 0);
    auto* splitter_layout = new QGridLayout(nullptr);
    splitter_layout->setContentsMargins(0, 0, 0, 0);
    splitter_layout->setSpacing(0);
    auto widget = new QWidget;
    widget->setLayout(splitter_layout);

    splitter->addWidget(widget);
    setupVariableView(splitter_layout);
    setupCodeView(splitter_layout);

    auto* tabbed_pane = setupTabbedPane();
    splitter->addWidget(tabbed_pane);
  }

  void TracerPanel::setupToolbar(QGridLayout* layout) {
    // Setup the main toolbar
    toolbar = new TracerToolBar(this);
    layout->addWidget(toolbar, 0, 0);
    connect(toolbar, &TracerToolBar::openCommand, this, &TracerPanel::popupStartCommandDialog);
  }

  void TracerPanel::TracerPanel::setupCodeView(QGridLayout* layout) {
    code_view = new CodeView(this);
    layout->addWidget(code_view, 0, 1, 1, 1);
  }

  void TracerPanel::setupVariableView(QGridLayout* layout) {
    variable_view = new VariableView(this);
    layout->addWidget(variable_view, 0, 0, 1, 1);
  }

  QTabWidget* TracerPanel::setupTabbedPane() {
    auto* stacked_pane = new QTabWidget(this);
    stacked_pane->setTabPosition(QTabWidget::South);
    stacked_pane->setTabShape(QTabWidget::Rounded);
    stacked_pane->setIconSize(QSize(16, 16));

    // Setup the tab where the log will be displayed
    auto& logger = tscl::logger.addHandler<QtLogHandler>("QtHandler");
    logger.tsType(tscl::timestamp_t::Partial);
    auto message = logger.getWidget();

    stacked_pane->addTab(message, "Message");
    stacked_pane->setTabIcon(0, QIcon(":/icons/menu-2-line.png"));

    // Setup the tab where the stack trace will be displayed
    stack_trace_view = new StackTraceView(this);
    stacked_pane->addTab(stack_trace_view, "Stack trace");
    stacked_pane->setTabIcon(1, QIcon(":/icons/stack-fill.png"));

    // Setup the tab where the loaded libraries will be displayed
    auto libs = new QTextEdit(this);
    stacked_pane->addTab(libs, "Loaded libraries");
    stacked_pane->setTabIcon(2, QIcon(":/icons/list-settings-line.png"));

    return stacked_pane;
  }

  void TracerPanel::popupStartCommandDialog() {
    auto* dialog = new CommandDialog(this);
    dialog->setModal(true);
    if (dialog->exec() == QDialog::Accepted) {
      auto command = dialog->getCommand();
      tscl::logger("Starting command '" + command.toStdString() + "'", tscl::Log::Information);
      code_view->openFile(command);
    }
  }
}// namespace ldb::gui

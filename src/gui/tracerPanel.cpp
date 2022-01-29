#include "tracerPanel.h"
#include "logWidget.h"
#include <QLabel>
#include <QProgressBar>
#include <QTabWidget>
#include <QTextEdit>
#include <QToolBar>
#include <QToolButton>
#include <tscl.hpp>


namespace ldb::gui {
  TracerPanel::TracerPanel(QWidget* parent) : QWidget(parent) {
    QGridLayout* layout = new QGridLayout(nullptr);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);

    setupToolbar(layout);
    setupVariableView(layout);
    setupCodeView(layout);
    setupTabbedPane(layout);
  }

  void TracerPanel::setupToolbar(QGridLayout* layout) {
    // Setup the main toolbar
    toolbar = new TracerToolBar(this);
    layout->addWidget(toolbar, 0, 0, 1, 2);
  }

  void TracerPanel::TracerPanel::setupCodeView(QGridLayout* layout) {
    code_view = new CodeView(this);
    layout->addWidget(code_view, 1, 1, 1, 1);
  }

  void TracerPanel::setupVariableView(QGridLayout* layout) {
    variable_view = new VariableView(this);
    layout->addWidget(variable_view, 1, 0, 2, 1);
  }

  void TracerPanel::setupTabbedPane(QGridLayout* layout) {
    auto* stacked_pane = new QTabWidget(this);
    stacked_pane->setTabPosition(QTabWidget::North);
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

    // Add the tab widget to the layout
    layout->addWidget(stacked_pane, 2, 1, 1, 1);
  }

  void TracerPanel::updateAll() {
    // Update all the view components
    toolbar->update(process_tracer.get());
    variable_view->update(process_tracer.get());
    code_view->update(process_tracer.get());
    stack_trace_view->update(process_tracer.get());
  }
}// namespace ldb::gui

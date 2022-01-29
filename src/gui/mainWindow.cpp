#include "mainWindow.h"
#include <QFile>
#include <QGridLayout>
#include <QMenuBar>
#include <QProgressBar>
#include <QWidget>

namespace ldb::gui {
  MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    showMaximized();
    setupMenuBar();
    setupTracerPanel();
  }

  void MainWindow::setupMenuBar() {
    QMenuBar* menu_bar = new QMenuBar(this);
    // File menu for loading new program and quitting
    QMenu* file_menu = menu_bar->addMenu("File");
    QAction* load_action = file_menu->addAction("Load program");
    QAction* quit_action = file_menu->addAction("Exit");

    // View menu, for showing/hiding components from the tracer panel
    QMenu* view_menu = menu_bar->addMenu("View");
    QAction* action_show_tracer = view_menu->addAction("Show variables");
    action_show_tracer->setCheckable(true);
    action_show_tracer->setChecked(true);

    QAction* action_show_graph = view_menu->addAction("Show source code");
    action_show_graph->setCheckable(true);
    action_show_graph->setChecked(true);

    QAction* action_show_stack = view_menu->addAction("Show stack trace");
    action_show_stack->setCheckable(true);
    action_show_stack->setChecked(true);

    QAction* action_show_loaded_libs = view_menu->addAction("Show loaded libraries");
    action_show_loaded_libs->setCheckable(true);
    action_show_loaded_libs->setChecked(true);

    QAction* action_show_toolbar = view_menu->addAction("Show toolbar");
    action_show_toolbar->setCheckable(true);
    action_show_toolbar->setChecked(true);

    auto* action_about = new QAction("About");
    menu_bar->addAction(action_about);

    auto* action_help = new QAction("Help");
    menu_bar->addAction(action_help);

    setMenuBar(menu_bar);
  }

  void MainWindow::setupTracerPanel() {
    tracer_panel = new TracerPanel(this);
    setCentralWidget(tracer_panel);
  }
}// namespace ldb::gui

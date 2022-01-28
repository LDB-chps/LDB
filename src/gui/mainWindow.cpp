//
// Created by thukisdo on 28/01/2022.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MainWindow.h" resolved

#include "mainWindow.h"
#include <QProgressBar>
#include <QWidget>
#include <QGridLayout>
#include <QMenuBar>
#include <QFile>

namespace ldb::gui {
  MainWindow::MainWindow(QWidget *parent) :
          QMainWindow(parent) {
    showMaximized();
    setupMenuBar();
    setupTracerPanel();
  }

  void MainWindow::setupMenuBar() {
    QMenuBar *menu_bar = new QMenuBar(this);
    QMenu *file_menu = menu_bar->addMenu("File");
    QMenu *view_menu = menu_bar->addMenu("View");
    QMenu *help_menu = menu_bar->addMenu("Help");
    QMenu *about_menu = menu_bar->addMenu("About");
    setMenuBar(menu_bar);
  }

  void MainWindow::setupTracerPanel() {
    tracer_panel = new TracerPanel(this);
    setCentralWidget(tracer_panel);
  }
} // ldb::gui

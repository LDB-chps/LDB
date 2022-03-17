#include "MainWindow.h"
#include <QFile>
#include <QGridLayout>
#include <QMenuBar>
#include <QMessageBox>
#include <QWidget>

namespace ldb::gui {
  MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    showMaximized();
    setupTracerPanel();
    setupMenuBar();
  }

  void MainWindow::refreshCSS() {
    QFile stylesheet_file("./my.qss");
    stylesheet_file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(stylesheet_file.readAll());
    qApp->setStyleSheet(styleSheet);
  }

  void MainWindow::setupMenuBar() {
    QMenuBar* menu_bar = new QMenuBar(this);
    // File menu for loading new program and quitting
    QMenu* file_menu = menu_bar->addMenu("File");
    QAction* refresh_css = file_menu->addAction("Refresh CSS");
    connect(refresh_css, &QAction::triggered, this, &MainWindow::refreshCSS);

    QAction* load_action =
            file_menu->addAction(QIcon(":/icons/folder-open-fill.png"), "Start command");
    connect(load_action, &QAction::triggered, tracer_panel, &TracerPanel::displayCommandDialog);
    QAction* quit_action = file_menu->addAction("Exit");
    connect(quit_action, &QAction::triggered, this, &QMainWindow::close);

    auto* action_about = new QAction("About");
    menu_bar->addAction(action_about);
    connect(action_about, &QAction::triggered, this, &MainWindow::startAboutPopup);

    auto* action_help = new QAction("Help");
    menu_bar->addAction(action_help);

    setMenuBar(menu_bar);
  }

  void MainWindow::startAboutPopup() {
    QMessageBox::about(this, "About",
                       "LDB is a graphical debugger project.\nThis project was made for the "
                       "AISE course for the M1CHPS at the University of Versailles "
                       "St-Quentin.\n\nAuthors: Ugo "
                       "Battiston, Mathys Jam");
  }

  void MainWindow::setupTracerPanel() {
    tracer_panel = new TracerPanel(this);
    setCentralWidget(tracer_panel);
  }
}// namespace ldb::gui

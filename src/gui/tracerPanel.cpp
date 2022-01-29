#include "tracerPanel.h"
#include <QLabel>
#include <QProgressBar>
#include <QTextEdit>
#include <QToolBar>
#include <QToolButton>


namespace ldb::gui {
  TracerPanel::TracerPanel(QWidget* parent) : QWidget(parent) {
    QGridLayout* layout = new QGridLayout(nullptr);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);

    QProgressBar* progressBar = new QProgressBar(this);
    progressBar->setRange(0, 0);
    setupToolbar(layout);

    VariableView* variableView = new VariableView(this);
    layout->addWidget(variableView, 1, 0, 2, 1);
    layout->addWidget(new QTextEdit("Code viewer"), 1, 1, 1, 1);
    // layout->addWidget(new QTextEdit("Stack"), 2, 1, 1, 1);
    layout->addWidget(progressBar, 2, 1, 1, 1);
  }

  void TracerPanel::setupToolbar(QGridLayout* layout) {
    QToolBar* toolbar = new QToolBar(this);
    toolbar->setIconSize(QSize(16, 16));
    toolbar->addAction(QIcon(":/icons/folder-open-fill.png"), "Open");
    toolbar->addSeparator();
    toolbar->addWidget(new QLabel("Program: <placeholder>"));
    toolbar->addAction(QIcon(":/icons/skip-back-fill.png"), "Reset");
    toolbar->addAction(QIcon(":/icons/play-fill.png"), "Play");
    toolbar->addAction(QIcon(":/icons/stop-fill.png"), "Stop");
    toolbar->addAction(QIcon(":/icons/skip-forward-fill.png"), "Finish");
    toolbar->addSeparator();
    toolbar->addWidget(new QLabel("Current file: <placeholder>"));
    toolbar->addWidget(new QLabel("PID: <placeholder>"));
    layout->addWidget(toolbar, 0, 0, 1, 2);
  }

  void TracerPanel::TracerPanel::setupCodeView() {}

  void TracerPanel::setupVariableView() {}

  void TracerPanel::setupStackedPane() {}
}// namespace ldb::gui

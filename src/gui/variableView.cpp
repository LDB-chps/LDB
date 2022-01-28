//
// Created by thukisdo on 28/01/2022.
//

// You may need to build the project (run Qt uic code generator) to get "ui_variableView.h" resolved

#include "variableView.h"
#include <QLabel>
#include <QGridLayout>
namespace ldb::gui {
  VariableView::VariableView(QWidget *parent) :
          QFrame(parent) {
    setFrameStyle(QFrame::Panel | QFrame::Sunken);
    auto *layout = new QGridLayout(this);
    layout->addWidget(new QLabel("Variable View"));
  }

} // ldb::gui

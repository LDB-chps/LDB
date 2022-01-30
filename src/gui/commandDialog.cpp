#include "commandDialog.h"
#include <QFileDialog>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPushButton>

namespace ldb::gui {
  CommandDialog::CommandDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Select a command");

    layout = new QGridLayout;
    setLayout(layout);

    // Setup command line edit and open folder button
    auto* label_command = new QLabel("Command: ");
    layout->addWidget(label_command, 0, 0);
    command = new QLineEdit();
    layout->addWidget(command, 0, 1, 1, 2);
    // Add a button for folder opening
    auto* open_folder = new QPushButton(QIcon(":/icons/folder-open-fill.png"), "");
    connect(open_folder, &QPushButton::clicked, this, &CommandDialog::openFileDialog);
    layout->addWidget(open_folder, 0, 3);

    // Setup a line edit for command arguments
    auto* label_args = new QLabel("Arguments: ");
    layout->addWidget(label_args, 1, 0);
    args = new QLineEdit();
    layout->addWidget(args, 1, 1, 1, 3);

    // Line separator between input and confirmation buttons
    QFrame* line_separator = new QFrame();
    line_separator->setFrameShape(QFrame::HLine);
    line_separator->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line_separator, 2, 0, 1, 4);

    // Add the buttons in a subwidget and add it to the layout
    // This improves the layout of the dialog
    QWidget* buttons = new QWidget;
    QHBoxLayout* buttons_layout = new QHBoxLayout;
    buttons->setLayout(buttons_layout);
    layout->addWidget(buttons, 3, 0, 1, 4);

    // Connect the exit and abort buttons to the slots provided by the parent class
    auto* abort = new QPushButton("Abort");
    connect(abort, &QPushButton::clicked, this, &CommandDialog::reject);

    auto* ok = new QPushButton(QIcon(":/icons/play-fill.png"), "Start");
    connect(ok, &QPushButton::clicked, this, &CommandDialog::accept);
    // We want buttons to be aligned to the right side of the dialog
    buttons_layout->addStretch();
    buttons_layout->addWidget(abort);
    buttons_layout->addWidget(ok);
  }

  QString CommandDialog::getCommand() const {
    return command->text();
  }

  QString CommandDialog::getArgs() const {
    return args->text();
  }

  void CommandDialog::openFileDialog() {
    // Open a file dialog for the user to select a command
    QString file_name = QFileDialog::getOpenFileName(this, "Open file", "", "All files (*.*)");
    if (!file_name.isEmpty()) { command->setText(file_name); }
  }
}// namespace ldb::gui
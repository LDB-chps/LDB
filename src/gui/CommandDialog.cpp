#include "CommandDialog.h"
#include <QFileDialog>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>


namespace ldb::gui {
  CommandDialog::CommandDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Select a command");

    layout = new QVBoxLayout;
    setLayout(layout);


    // Inputs are added to a subwidget
    auto* input_layout = new QGridLayout;
    QWidget* input = new QWidget;
    input->setLayout(input_layout);
    layout->addWidget(input);

    // Setup command line edit and open folder button
    auto* label_command = new QLabel("Command: ");
    input_layout->addWidget(label_command, 0, 0);
    command = new QLineEdit();
    input_layout->addWidget(command, 1, 0, 1, 1);
    // Add a button for folder opening
    auto* open_folder = new QPushButton(QIcon(":/icons/folder-open-fill.png"), "");
    connect(open_folder, &QPushButton::clicked, this, &CommandDialog::openFileDialog);
    input_layout->addWidget(open_folder, 1, 2, 1, 1, Qt::AlignRight);

    // Setup a line edit for command arguments
    auto* label_args = new QLabel("Arguments: ");
    input_layout->addWidget(label_args, 2, 0);
    args = new QTextEdit();
    input_layout->addWidget(args, 3, 0, 1, 3);

    // Line separator between input and confirmation buttons
    QFrame* line_separator = new QFrame();
    line_separator->setFrameShape(QFrame::HLine);
    line_separator->setFrameShadow(QFrame::Sunken);
    layout->addStretch();
    layout->addWidget(line_separator);

    // Add the buttons in a subwidget and add it to the layout
    // This improves the layout of the dialog
    QWidget* buttons = new QWidget;
    QHBoxLayout* buttons_layout = new QHBoxLayout;
    buttons->setLayout(buttons_layout);
    layout->addWidget(buttons);

    // Connect the exit and abort buttons to the slots provided by the parent class
    auto* button_cancel = new QPushButton("Cancel");
    connect(button_cancel, &QPushButton::clicked, this, &CommandDialog::reject);

    auto* ok = new QPushButton(QIcon(":/icons/play-fill.png"), "Start");
    connect(ok, &QPushButton::clicked, this, &CommandDialog::accept);
    // We want buttons to be aligned to the right side of the dialog
    buttons_layout->addStretch();
    buttons_layout->addWidget(button_cancel);
    buttons_layout->addWidget(ok);
  }

  QString CommandDialog::getCommand() const {
    return command->text();
  }

  QString CommandDialog::getArgs() const {
    return args->toPlainText();
  }

  void CommandDialog::openFileDialog() {
    // Open a file dialog for the user to select a command
    QString file_name = QFileDialog::getOpenFileName(this, "Open file");
    if (!file_name.isEmpty()) { command->setText(file_name); }
  }
}// namespace ldb::gui
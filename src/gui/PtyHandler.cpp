#include "PtyHandler.h"
#include <QGridLayout>
#include <QThread>
#include <iostream>

namespace ldb::gui {


  PtyHandler::PtyHandler(QWidget* parent, int fd) : QWidget(parent), pty_fd(fd), thread(nullptr) {

    auto* layout = new QGridLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    output = new QTextEdit();
    output->setReadOnly(true);
    output->setFontPointSize(11);
    output->setTextColor(Qt::white);
    output->document()->setMaximumBlockCount(250);
    layout->addWidget(output, 0, 0, 1, 2);
    connect(output, &QTextEdit::textChanged, this, &PtyHandler::scrollToBottom);

    input = new QLineEdit();
    layout->addWidget(input, 1, 0);
    connect(input, &QLineEdit::returnPressed, this, &PtyHandler::sendInput);

    button_send = new QPushButton("Input");
    layout->addWidget(button_send, 1, 1);
    connect(button_send, &QPushButton::clicked, this, &PtyHandler::sendInput);

    // No need to create a thread if the file descriptors are invalid
    if (pty_fd < 0) {
      input->setDisabled(true);
      button_send->setDisabled(true);
      return;
    }
    thread = QThread::create(&PtyHandler::workerLoop, this);
    thread->start();
  }

  void PtyHandler::scrollToBottom() {
    output->ensureCursorVisible();
  }

  void PtyHandler::sendInput() {
    if (pty_fd < 0) { return; }

    auto text = input->text();
    // No need to send empty lines
    if (text.isEmpty()) { return; }

    // Send the input to the pipe
    // Always append a newline to the input since the pty is line buffered
    text += "\n";
    write(pty_fd, text.toUtf8().data(), text.size());
    output->append(text);
    output->ensureCursorVisible();
    input->clear();
  }

  void PtyHandler::reassignTo(int fd) {
    // Terminate the thread to avoid concurrency issues
    // The thread may have not been allocated yet
    if (thread and thread->isRunning()) { thread->terminate(); }

    this->pty_fd = fd;

    // Disable the widgets if the file descriptors are invalid
    input->setDisabled(fd < 0);
    button_send->setDisabled(fd < 0);

    if (fd < 0) return;

    output->clear();

    if (pty_fd < 0) { return; }

    thread = QThread::create(&PtyHandler::workerLoop, this);
    thread->start();
  }

  void PtyHandler::appendOutput(QString text) {
    output->moveCursor(QTextCursor::End);
    output->insertPlainText(text);
    output->moveCursor(QTextCursor::End);
  }

  void PtyHandler::workerLoop() {
    bool done = false;
    char buffer[1024];

    while (not done) {
      // Block until data is available
      long bytes_read = read(pty_fd, buffer, sizeof(buffer) - 1);
      buffer[bytes_read] = '\0';

      // Since we are running in another thread, we must append using signals to avoid sigsev
      if (bytes_read > 0) {
        QMetaObject::invokeMethod(this, "appendOutput", Qt::QueuedConnection,
                                  Q_ARG(QString, QString::fromUtf8(buffer, bytes_read)));
      } else if (bytes_read <= 0) {
        // If the pty closes, we just stop the thread
        done = true;
      }
    }
  }

}// namespace ldb::gui

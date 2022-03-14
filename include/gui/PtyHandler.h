#pragma once

#include <QLineEdit>
#include <QTextEdit>
#include <QWidget>
#include <QPushButton>


namespace ldb::gui {

  /**
   * @brief Reads from a pipe and displays the output in a QPlainTextEdit
   * Also provides a text edit for inputting into a pipe
   *
   * This class is use for communication with the process that is running in the background.
   */
  class PtyHandler : public QWidget {
  public:
    PtyHandler(QWidget* parent, int fd);
    void setPTy(int fd);

  public slots:
    void sendInput();
    void scrollToBottom();

  private:

    QThread *thread;
    void workerLoop();

    int pty_fd;

    QTextEdit* output;
    QLineEdit* input;
    QPushButton* button_send;
  };
}// namespace ldb::gui

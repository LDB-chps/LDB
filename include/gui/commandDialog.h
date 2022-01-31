#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QVBoxLayout>
namespace ldb::gui {
  /*
   * @brief A dialog box for entering a command and its argument
   */
  class CommandDialog : public QDialog {
    Q_OBJECT
  public:
    /**
     * @brief Creates a new blank dialog box
     * @param parent
     */
    CommandDialog(QWidget* parent = nullptr);

    /**
     * @brief Returns the command entered by the user
     * @return A QString containing the command. The command is not guaranteed to be correct
     */
    QString getCommand() const;

    /**
     * @brief Returns the command entered by the user
     * @return A QString containing the command. The command is not guaranteed to be correct
     */
    QString getArgs() const;

  public slots:
    /**
     * @brief Open a file dialog for the user to select the command to start
     */
    void openFileDialog();

  private:
    QVBoxLayout* layout;
    QLineEdit* command;
    QTextEdit* args;
  };

}// namespace ldb::gui
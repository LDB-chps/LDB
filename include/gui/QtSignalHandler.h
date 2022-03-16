#pragma once
#include "Process.h"
#include "SignalHandler.h"
#include <QtCore/QObject>
#include <condition_variable>

namespace ldb::gui {

  class QtSignalHandler : public QObject, public SignalHandler {
    Q_OBJECT
  public:
    explicit QtSignalHandler(Process* process);
    ~QtSignalHandler() override;

    SignalEvent handleEvent(const SignalEvent& event) override;

    void reset() override;

  signals:

    void signalReceived(SignalEvent event);
    void processExited();
    void ignoredEvent(SignalEvent event);

  private slots:

    void resumeTracee(SignalEvent event);

  private:
    void workerLoop();
    QThread* worker_thread;
  };

}// namespace ldb::gui

#pragma once
#include "Process.h"
#include "SignalHandler.h"
#include <QtCore/QObject>
#include <condition_variable>

namespace ldb::gui {

  class QtSignalHandler : public QObject, public SignalHandler {
    Q_OBJECT
  public:
    explicit QtSignalHandler(Process* process, BreakPointHandler* bph);
    ~QtSignalHandler() override;

    SignalEvent handleEvent(const SignalEvent& event) override;

    void reset(Process* p) override;

    void mute() override;
    void unmute() override;

  signals:

    void signalReceived(SignalEvent event);
    void processExited();
    void ignoredEvent(SignalEvent event);

  private slots:

    void resumeTracee(SignalEvent event);

  private:
    void stopThread();

    std::mutex mutex;
    std::atomic<bool> worker_waiting;
    std::atomic<bool> worker_exit;

    void workerLoop();
    QThread* worker_thread;
    std::condition_variable update_cv;
  };

}// namespace ldb::gui

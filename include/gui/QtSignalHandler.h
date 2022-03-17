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

    void reset(Process* p) override;

    void mute() override {
      is_muted = true;
      update_cv.notify_one();
      std::condition_variable cv;
      cv.wait(worker_waiting);
    }

    virtual void unmute() {
      is_muted = false;
    }

  signals:

    void signalReceived(SignalEvent event);
    void processExited();
    void ignoredEvent(SignalEvent event);

  private slots:

    void resumeTracee(SignalEvent event);

  private:
    std::atomic<bool> worker_waiting;
    void workerLoop();
    QThread* worker_thread;
    std::condition_variable update_cv;
  };

}// namespace ldb::gui

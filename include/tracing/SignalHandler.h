#pragma once
#include "Process.h"
#include <condition_variable>
#include <thread>

namespace ldb::gui {

  class SignalEvent {
  public:
    SignalEvent(Signal signal, Process::Status status, bool is_ignored, bool is_fatal);

    Signal getSignal() const {
      return signal;
    }

    Process::Status getStatus() const {
      return status;
    }

    bool isIgnored() const {
      return is_ignored;
    }

    bool isFatal() const {
      return is_fatal;
    }

  private:
    Signal signal;
    Process::Status status;
    bool is_ignored;
    bool is_fatal;
  };

  class SignalHandler {
  public:
    SignalHandler(Process* process);
    virtual ~SignalHandler();

    void setProcess(Process* process);

    SignalEvent nextSignal();
    virtual void handleSignal(SignalEvent& event);

    void beginWorker();
    void endWorker();

  private:
    SignalEvent makeSignalEvent(int signal);

    std::thread worker;
    std::condition_variable worker_cv;
    std::atomic<bool> worker_running;

    std::vector<bool> ignored_signals;

    Process* process;
  };

}// namespace ldb::gui

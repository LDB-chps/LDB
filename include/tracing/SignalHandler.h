#pragma once
#include "Process.h"
#include <condition_variable>
#include <thread>

namespace ldb {

  class SignalEvent {
  public:
    SignalEvent(Signal signal, Process::Status status, bool is_ignored, bool is_fatal) noexcept
        : signal(signal), status(status), is_ignored(is_ignored), is_fatal(is_fatal) {}

    static const SignalEvent Unknown;
    static const SignalEvent None;

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
    virtual ~SignalHandler() = default;

    // Called when the process is restarted
    // By default it does nothing, but can be used by subclasses to reset
    // their internal state
    virtual void reset(){};

    SignalEvent nextSignal();
    virtual SignalEvent handleEvent(const SignalEvent& event);

    void setIgnored(Signal signal, bool ignored);

  protected:
    std::vector<bool> ignored_signals;
    Process* process;
  };

}// namespace ldb

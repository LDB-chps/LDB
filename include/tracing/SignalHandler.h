#pragma once
#include "Process.h"
#include <condition_variable>
#include <optional>
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

    /**
     * @brief This signal is called whenever the process is restarted
     * This avoids the need to create a new SignalHandler for the new process
     * @param p A pointer to the new process
     */
    virtual void reset(Process* p) {
      process = p;
    }

    bool isMuted() {
      return is_muted;
    }

    virtual void mute() {
      is_muted = true;
    }

    virtual void unmute() {
      is_muted = false;
    }

    SignalEvent waitEvent();
    std::optional<SignalEvent> waitEvent(size_t usec);

    void setIgnored(Signal signal, bool ignored);

  protected:
    virtual SignalEvent handleEvent(const SignalEvent& event);
    SignalEvent makeEventFromSignal(int signal);

    /**
     * @brief Wait for a signal to be received. Throws an exception on error (i.e the process was
     * killed)
     *
     * @param timeout The timeout in useconds
     * If set to 0, the function will wait indefinitely, and is guaranteed to return a valid
     * event, or throw if it can't.
     *
     * Else, if timeout is set to a valid value, the function will return a valid event if it
     * received one in the given time. Otherwise, it will return std::nullopt. If an errors occurs,
     * the function will throw.
     *
     * @return A valid event if one was received, or std::nullopt if the timeout was reached.
     * Guaranteed to return a valid event if timeout is nullptr and no error occurs.
     */
    std::optional<SignalEvent> pollEvent(size_t utimeout);

    std::atomic<bool> is_muted;
    std::vector<bool> ignored_signals;
    Process* process;
  };

}// namespace ldb

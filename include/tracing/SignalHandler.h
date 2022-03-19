#pragma once
#include "Process.h"
#include <condition_variable>
#include <optional>
#include <thread>
#include "BreakPointHandler.h"

namespace ldb {

  /**
   * @brief Represents an event received by the signal handler.
   * Groups the signal that was received, the status of the process after the signal, and a flag to
   * check if the event is ignored and fatal
   */
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

    // Set to true if the signal is ignored, meaning that the process was automatically resumed
    bool is_ignored;
    // Set to true if the signal is fatal, meaning that the process was terminated
    bool is_fatal;
  };

  /**
   * @brief Handles signals from the process
   *
   * Base class for signal handlers that are responsible for catching and handling signals. This
   * class is also responsible for breakpoints restoration.
   */
  class SignalHandler {
  public:
    /**
     * @brief Constructs a SignalHandler for the given process and breakpoint handler
     * @param process
     * @param bph
     */
    SignalHandler(Process* process, BreakPointHandler* bph);
    virtual ~SignalHandler() = default;

    /**
     * @brief This signal is called whenever the process is restarted
     * This avoids the need to create a new SignalHandler for the new process
     * @param p A pointer to the new process
     * @param bph A pointer to the new breakpoint handler
     */
    virtual void reset(Process* p, BreakPointHandler* bph) {
      process = p;
      breakpoint_handler = bph;
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

    /**
     * @brief Wait for a signal to be received, or throw on error
     * @return The signal that was received
     */
    SignalEvent waitEvent();

    /**
     * @brief Wait for @usec microseconds for a signal, and return after one was received or
     * reaching timeout
     * @param usec
     * @return
     */
    std::optional<SignalEvent> waitEvent(size_t usec);

    void setIgnored(Signal signal, bool ignored);

  protected:
    virtual SignalEvent handleEvent(const SignalEvent& event);
    SignalEvent makeEventFromSignal(int signal);

    /**
     * @brief Wait for a signal to be received. Throws an exception on error (i.e the process was
     * killed)
     *
     * @param utimeout The timeout in useconds
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
    BreakPointHandler* breakpoint_handler;
    Process* process;
  };

}// namespace ldb

#include "SignalHandler.h"
#include <cerrno>
#include <sys/ptrace.h>
#include <tscl.hpp>

namespace ldb {

  const SignalEvent SignalEvent::Unknown{Signal::kUnknown, Process::Status::kUnknown, true, false};

  const SignalEvent SignalEvent::None{Signal::kSignalCount, Process::Status::kUnknown, true, false};

  SignalHandler::SignalHandler(Process* process, BreakPointHandler* bph)
      : process(process), is_muted(false), breakpoint_handler(bph) {
    ignored_signals.resize(static_cast<size_t>(Signal::kSignalCount));
    ignored_signals.assign(ignored_signals.size(), false);

    setIgnored(Signal::kSIGCHLD, true);
    setIgnored(Signal::kSIGALRM, true);
    setIgnored(Signal::kSIGCONT, true);
    setIgnored(Signal::kSIGURG, true);
    setIgnored(Signal::kSIGWINCH, true);
  }

  void SignalHandler::setIgnored(Signal signal, bool ignored) {
    ignored_signals[static_cast<size_t>(signal)] = ignored;
  }

  SignalEvent SignalHandler::waitEvent() {
    if (is_muted) return SignalEvent::None;
    std::optional<SignalEvent> e = pollEvent(0);
    return handleEvent(*e);
  }

  std::optional<SignalEvent> SignalHandler::waitEvent(size_t usec) {
    if (is_muted) return SignalEvent::None;
    std::optional<SignalEvent> e = pollEvent(usec);

    if (not e) return std::nullopt;

    return handleEvent(*e);
  }

  std::optional<SignalEvent> SignalHandler::pollEvent(size_t usec) {
    size_t slept = 0;
    size_t delta = std::min(10000UL, usec);
    int res = 0;
    int status = 0;

    if (usec == 0) res = waitpid(process->getPid(), &status, 0);
    else
      do {
        res = waitpid(process->getPid(), &status, WNOHANG);
        usleep(delta);
        slept += delta;
      } while (slept <= usec and res == 0 and not is_muted);


    if (res == 0) {
      if (usec == 0) throw std::runtime_error("waitpid() failed");
      return std::nullopt;
    } else if (res < 0) {
      return SignalEvent{Signal::kSIGQUIT, Process::Status::kDead, true, false};
    }

    int signal = 0;
    if (WIFSTOPPED(status)) {
      signal = WSTOPSIG(status);
    } else if (WIFSIGNALED(status)) {
      signal = WTERMSIG(status);
    } else if (WIFEXITED(status)) {
      signal = SIGQUIT;
    }
    return makeEventFromSignal(signal);
  }

  SignalEvent SignalHandler::makeEventFromSignal(int signal) {

    Process::Status new_status = Process::Status::kUnknown;

    // Failed to find the process
    if (signal == SIGCONT) new_status = Process::Status::kRunning;
    else if (signal == SIGQUIT)
      new_status = Process::Status::kExited;
    // Handle catchable signals
    else if (signal == SIGTRAP or signal == SIGSTOP or signal == SIGTSTP or signal == SIGTTIN or
             signal == SIGTTOU or signal == SIGCHLD or signal == SIGALRM or signal == SIGURG or
             signal == SIGWINCH)
      new_status = Process::Status::kStopped;
    else
      new_status = Process::Status::kKilled;
    process->updateStatus(new_status);
    return {static_cast<Signal>(signal), new_status, false,
            new_status == Process::Status::kKilled or new_status == Process::Status::kExited or
                    new_status == Process::Status::kDead};
  }

  SignalEvent SignalHandler::handleEvent(const SignalEvent& event) {
    if (ignored_signals[static_cast<size_t>(event.getSignal())]) {
      int res = ptrace(PTRACE_CONT, process->getPid(), nullptr, nullptr);
      // If we failed to resume the process, we consider the signal as non-ignored
      if (res == -1) {
        tscl::logger("Failed to continue process: current thread might not be attached",
                     tscl::Log::Error);
      }
      return {event.getSignal(), event.getStatus(), false, false};
    }
    return event;
  }
}// namespace ldb

#include "SignalHandler.h"
#include <sys/ptrace.h>
#include <tscl.hpp>

namespace ldb {

  const SignalEvent SignalEvent::Unknown{Signal::kUnknown, Process::Status::kUnknown, true, false};

  const SignalEvent SignalEvent::None{Signal::kSignalCount, Process::Status::kUnknown, true, false};

  SignalHandler::SignalHandler(Process* process) : process(process) {
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

  SignalEvent SignalHandler::nextSignal() {
    int s;
    int res = waitpid(process->getPid(), &s, WCONTINUED);

    if (res == 0) return SignalEvent::None;

    Process::Status new_status = Process::Status::kUnknown;
    Signal signal = Signal::kUnknown;

    // Failed to find the process
    if (res == -1) {
      new_status = Process::Status::kDead;
      signal = Signal::kSIGABRT;
    } else if (WIFCONTINUED(s)) {
      signal = Signal::kSIGCONT;
      new_status = Process::Status::kRunning;
    } else if (WIFEXITED(s)) {
      new_status = Process::Status::kExited;
      signal = Signal::kSIGQUIT;
    }
    // Handle catchable signals
    else if (WIFSTOPPED(s)) {
      // The following signals are considered to be stopping by default
      // (Handling ignored signals is done at a higher level)
      if (WSTOPSIG(s) == SIGTRAP or WSTOPSIG(s) == SIGSTOP or WSTOPSIG(s) == SIGTSTP or
          WSTOPSIG(s) == SIGTTIN or WSTOPSIG(s) == SIGTTOU or WSTOPSIG(s) == SIGCHLD or
          WSTOPSIG(s) == SIGALRM or WSTOPSIG(s) == SIGCONT or WSTOPSIG(s) == SIGURG or
          WSTOPSIG(s) == SIGWINCH)
        new_status = Process::Status::kStopped;
      // The remaining signals are considered as fatal
      // A better options would be to fetch the handled signals from the process
      // and check them this way
      else
        new_status = Process::Status::kKilled;
      signal = static_cast<Signal>(WSTOPSIG(s));
    }
    // Handle signals that can't be caught
    else if (WIFSIGNALED(s)) {
      new_status = Process::Status::kKilled;
      signal = static_cast<Signal>(WTERMSIG(s));
    }
    process->updateStatus(new_status);
    return handleEvent({signal, new_status, false, false});
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

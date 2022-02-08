#include "process.h"
#include <csignal>
#include <iostream>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace ldb {


  Process::Process(pid_t pid) : pid(pid) {}

  std::unique_ptr<Process> Process::fromCommand(const std::string& command,
                                                const std::string& args) {
    auto res = std::make_unique<Process>(fork());
    if (res->getPid() == 0) {
      ptrace(PTRACE_TRACEME, 0, nullptr, nullptr);
      // Since we switch to a new process, no need to worry about this object state anymore
      execlp(command.c_str(), command.c_str(), args.c_str(), nullptr);
      exit(1);
    }

    if (res->getPid() == -1) { throw std::runtime_error("Failed to fork"); }
    return res;
  }

  bool Process::isRunning() {
    // Check the status of the process
    int status;
    if (waitpid(pid, &status, WNOHANG) == -1) { return false; }
    return WIFSTOPPED(status) == 0;
  }

  bool Process::resume() {
    return ptrace(PTRACE_CONT, pid, nullptr, nullptr) == 0;
  }

  bool Process::pause() {
    return ::kill(pid, SIGSTOP) == 0;
  }

  bool Process::kill() {
    if (::kill(pid, SIGKILL) == 0) {
      waitpid(pid, nullptr, 0);
      return true;
    }
    return false;
  }

  bool Process::isAlive() {
    // The process may be waiting for us to sync up, so we try to wait for it
    waitpid(pid, nullptr, WNOHANG);

    // Send a kill signal and check errno for the "no such process" error
    return ::kill(pid, 0) != -1 or errno == ESRCH;
  }

  void Process::wait() {
    waitpid(pid, nullptr, 0);
  }
}// namespace ldb
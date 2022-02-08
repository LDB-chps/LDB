#include "process.h"
#include <csignal>
#include <iostream>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace ldb {


  Process::Process(pid_t pid) : pid(pid) {}

  Process Process::fromCommand(const std::string& command, const std::string& args) {
    Process res(fork());
    if (res.pid == 0) {
      ptrace(PTRACE_TRACEME, 0, nullptr, nullptr);
      // Since we switch to a new process, no need to worry about this object state anymore
      execlp(command.c_str(), command.c_str(), args.c_str(), nullptr);
      exit(1);
    }
    return res;
  }

  bool Process::resume() {
    return ptrace(PTRACE_CONT, pid, nullptr, nullptr) == 0;
  }

  bool Process::pause() {
    return ::kill(pid, SIGSTOP) == 0;
  }

  bool Process::kill() {
    return ptrace(PTRACE_KILL, pid, nullptr, nullptr) == 0;
  }

  bool Process::isAlive() {
    // The process may be waiting for us to sync up, so we try to wait for it
    waitpid(pid, nullptr, WNOHANG);

    // Send a kill signal and check errno for the "no such process" error
    if (::kill(pid, 0) == -1 and errno != ESRCH) { return false; }
    return true;
  }

  void Process::wait() {
    waitpid(pid, nullptr, 0);
  }
}// namespace ldb
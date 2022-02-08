#include "process.h"
#include <csignal>
#include <iostream>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace ldb {


  std::unique_ptr<Process> Process::fromCommand(const std::string& command,
                                                const std::string& args) {
    auto res = std::make_unique<Process>(fork());
    if (res->getPid() == 0) {
      std::cout << "Child process forked !" << std::endl;
      ptrace(PTRACE_TRACEME, 0, nullptr, nullptr);
      std::cout << "Child process now traced !" << std::endl;
      // Since we switch to a new process, no need to worry about this object state anymore
      execlp(command.c_str(), command.c_str(), args.c_str(), nullptr);
      std::cout << "Child process exiting !" << std::endl;
      exit(1);
    }

    if (res->getPid() == -1) { throw std::runtime_error("Failed to fork"); }
    return res;
  }

  Process::Process(pid_t pid) : pid(pid) {}

  Process::~Process() {
    if (pid != -1) return;

    auto status = getStatus();
    if (status == Status::Running or status == Status::Stopped) kill();
  }

  Process::Status Process::getStatus() {
    int status;
    if (waitpid(pid, &status, 0) == -1) {
      if (errno == ESRCH) { return Status::Dead; }
    }
    if (WIFEXITED(status)) { return Status::Exited; }
    if (WIFSTOPPED(status)) { return Status::Stopped; }
    if (WIFSIGNALED(status)) { return Status::Killed; }
    return Status::Running;
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

  void Process::wait() {
    waitpid(pid, nullptr, 0);
  }
}// namespace ldb
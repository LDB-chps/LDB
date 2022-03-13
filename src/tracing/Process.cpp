#include "Process.h"
#include <csignal>
#include <fcntl.h>
#include <iostream>
#include <pty.h>
#include <sys/ptrace.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace ldb {


  Process::Process(pid_t pid) : pid(pid) {
    if (pid != -1) status = getStatus();
    master_fd = -1;
    slave_fd = -1;
  }

  Process::~Process() {
    if (pid != -1) return;

    if (status == Status::kRunning or status == Status::kStopped) kill();
    // Close all pipes if they are still open
    // No need to check for failure since those pipes cannot be used after we return from this
    // destructor
    close(master_fd);
    close(slave_fd);
  }

  std::unique_ptr<Process> Process::fromCommand(const std::string& command, const std::string& args,
                                                bool pipe_output) {
    // In the case where we want to pipe the output, we must creates the pipes before forking
    // Thus we temporarily init the process with a -1 pid
    auto res = Process::fork(pipe_output);

    if (res->getPid() == 0) {
      ptrace(PTRACE_TRACEME, 0, nullptr, nullptr);

      if (pipe_output) {
        // Redirect both stdout and stderr to the pipe
        dup2(res->slave_fd, STDOUT_FILENO);
        dup2(res->slave_fd, STDERR_FILENO);

        // Redirect the pipe to stdin
        dup2(res->slave_fd, STDIN_FILENO);
      }

      char* argv[3] = {const_cast<char*>(command.c_str()), "100", nullptr};

      execvp(command.c_str(), argv);
      std::cout << "Child process: exiting !" << std::endl;
      exit(1);
    }

    if (res->getPid() == -1) { throw std::runtime_error("Failed to fork"); }
    return res;
  }

  std::unique_ptr<Process> Process::fork(bool create_pipe) {
    auto res = std::make_unique<Process>(0);

    // We need to creates the pipes before forking
    if (create_pipe and openpty(&res->master_fd, &res->slave_fd, nullptr, nullptr, nullptr)) {

      throw std::runtime_error("Failed to create pipes");
    }
    termios tty_param;
    tcgetattr(res->master_fd, &tty_param);
    tty_param.c_lflag &= ~ECHO;
    tcsetattr(res->master_fd, TCSANOW, &tty_param);
    unlockpt(res->slave_fd);


    res->pid = ::fork();
    // Update the status after changing its pid
    res->getStatus();
    return res;
  }

  Process::Status Process::getStatus() {
    return status;
  }

  Process::Status Process::waitNextEvent() {
    int s;
    int res = waitpid(pid, &s, 0);
    // The status hasn't changed
    if (res == 0) return status;
    // Failed to find the process
    if (res == -1) {
      if (errno == ESRCH) {
        return status = Status::kDead;
      } else
        return status = Status::kUnknown;
    }
    if (WIFEXITED(s)) { return status = Status::kExited; }
    if (WIFSTOPPED(s)) { return status = Status::kStopped; }
    if (WIFSIGNALED(s)) { return status = Status::kKilled; }
    return Status::kRunning;
  }


  bool Process::resume() {
    bool res = ptrace(PTRACE_CONT, pid, nullptr, nullptr) == 0;
    if (res) { status = Status::kRunning; }
    return res;
  }

  bool Process::pause() {
    return ::kill(pid, SIGSTOP) == 0;
  }

  bool Process::kill() {
    if (::kill(pid, SIGKILL) == 0) {
      waitpid(pid, nullptr, 0);
      status = Status::kDead;
      return true;
    }
    return false;
  }

  void Process::wait() {
    waitpid(pid, nullptr, 0);
    getStatus();
  }

  bool Process::attach() {
    bool failure = ptrace(PTRACE_ATTACH, pid, nullptr, nullptr);
    is_attached = not failure;
    return is_attached;
  }
}// namespace ldb
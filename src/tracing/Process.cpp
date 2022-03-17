#include "Process.h"
#include <csignal>
#include <fcntl.h>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <pty.h>
#include <sys/ptrace.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace fs = std::filesystem;

namespace ldb {

  std::string signalToString(Signal signal) {
    switch (signal) {
      case Signal::kUnknown:
        return "Unknown signal";
      case Signal::kSIGHUP:
        return "sighup: Hangup (parent process ended)";
      case Signal::kSIGINT:
        return "sigint: Interrupt (Ctrl-C)";
      case Signal::kSIGQUIT:
        return "sigquit: Quit (core-dumped)";
      case Signal::kSIGILL:
        return "sigill: Illegal instruction (corrupted binary)";
      case Signal::kSIGTRAP:
        return "sigtrap: Trace/breakpoint trap";
      case Signal::kSIGABRT:
        return "sigabrt: Aborted";
      case Signal::kSIGBUS:
        return "sigbus: Bus error (Invalid memory access)";
      case Signal::kSIGFPE:
        return "sigfpe: Floating point exception";
      case Signal::kSIGKILL:
        return "sigkill: Killed";
      case Signal::kSIGUSR1:
        return "sigusr1: User-defined signal 1";
      case Signal::kSIGSEGV:
        return "sigsegv: Segmentation fault";
      case Signal::kSIGUSR2:
        return "sigusr2: User-defined signal 2";
      case Signal::kSIGPIPE:
        return "sigpipe: Write to pipe with no readers";
      case Signal::kSIGALRM:
        return "sigalrm: Alarm clock";
      case Signal::kSIGTERM:
        return "sigterm: Terminated";
      case Signal::kSIGSTKFLT:
        return "sigstkflt: Stack fault";
      case Signal::kSIGCHLD:
        return "sigchld: Child status has changed";
      case Signal::kSIGCONT:
        return "sigcont: Continued";
      case Signal::kSIGSTOP:
        return "sigstop: Stopped (pause)";
      case Signal::kSIGTSTP:
        return "sigtstp: Stopped (user)";
      case Signal::kSIGTTIN:
        return "sigttin: Stopped (tty input)";
      case Signal::kSIGTTOU:
        return "sigttou: Stopped (tty output)";
      case Signal::kSIGURG:
        return "sigurg: Urgent condition on socket";
      case Signal::kSIGXCPU:
        return "sigxcpu: CPU time limit exceeded";
      case Signal::kSIGXFSZ:
        return "sigxfsz: File size limit exceeded";
      case Signal::kSIGVTALRM:
        return "sigvtalrm: Virtual alarm clock";
      case Signal::kSIGPROF:
        return "sigprof: Profiling timer expired";
      case Signal::kSIGWINCH:
        return "sigwinch: Window size change";
      case Signal::kSIGIO:
        return "sigio: I/O now possible";
      case Signal::kSIGPWR:
        return "sigpwr: Power failure reset";
      case Signal::kSIGSYS:
        return "sigsys: Bad system call";
    }
    return "Unknown signal";
  }

  Process::Process(pid_t pid, ClosePolicy cp) : pid(pid), close_policy(cp) {
    if (pid != -1) status = getStatus();
    master_ptty = -1;
    slave_ptty = -1;
  }

  Process::~Process() {
    if (pid <= 0) return;

    // Close all ptty if they are still open
    if (master_ptty >= 0) close(master_ptty);
    if (slave_ptty >= 0) close(slave_ptty);

    if (close_policy == ClosePolicy::kKill) {
      // Kill the process
      ::kill(pid, SIGKILL);
      waitpid(pid, nullptr, 0);
    }
    // Detach the process
    else if (close_policy == ClosePolicy::kDetach and is_attached) {
      ::ptrace(PTRACE_DETACH, pid, nullptr, nullptr);
    } else if (close_policy == ClosePolicy::kWait) {
      // Wait for the process to exit
      waitpid(pid, nullptr, 0);
    }
  }

  // Mutexes are non-copyable, so we cannot use the default move operators
  Process::Process(Process&& other) noexcept : pid(-1), master_ptty(-1), slave_ptty(-1) {
    *this = std::move(other);
  }

  Process& Process::operator=(Process&& other) noexcept {

    // If we're replacing the current process with another one, we must first stop it
    // To avoid creating zombies
    if (pid != -1 and other.pid != pid) kill();

    std::scoped_lock<std::shared_mutex> lock(mutex);

    pid = other.pid;
    status = other.status;
    master_ptty = other.master_ptty;
    slave_ptty = other.slave_ptty;

    other.pid = -1;
    other.status = Status::kUnknown;
    other.master_ptty = -1;
    other.slave_ptty = -1;
    return *this;
  }


  std::unique_ptr<Process> Process::fromCommand(const std::string& command,
                                                const std::vector<std::string>& args,
                                                bool pipe_output, ClosePolicy close_policy) {

    if (not std::filesystem::exists(command)) return nullptr;
    const auto permissions = std::filesystem::status("file.txt").permissions();

    // In the case where we want to pipe the output, we must creates the pipes before forking
    // Thus we temporarily init the process with a -1 pid
    auto res = Process::fork(pipe_output, close_policy);

    if (res->getPid() == 0) {
      int start_flags = PTRACE_O_TRACEEXEC;
      // Prevent the child from becoming a zombie if the trqcer dies
      if (close_policy == ClosePolicy::kKill) start_flags |= PTRACE_O_EXITKILL;

      ptrace(PTRACE_TRACEME, 0, start_flags, nullptr);

      // Build a vector containing all the arguments
      std::vector<const char*> argv_c;
      argv_c.reserve(args.size() + 1);
      argv_c.push_back(command.c_str());

      for (auto& it : args) {
        // Do not add empty strings
        // This may lead to crash
        if (it.empty()) continue;
        argv_c.push_back(it.c_str());
      }
      argv_c.push_back(nullptr);

      if (pipe_output) {
        // Redirect both stdout and stderr to the pipe
        dup2(res->slave_ptty, STDOUT_FILENO);
        dup2(res->slave_ptty, STDERR_FILENO);

        // Redirect the pipe to stdin
        dup2(res->slave_ptty, STDIN_FILENO);
      }

      // Unfortunately, execvp does not respect constness
      // So we have to do a const_cast...
      // We really should manually copy the arguments properly,
      // But I don't want to spend too much time on that
      execv(command.c_str(), const_cast<char* const*>(argv_c.data()));
      throw std::runtime_error("Failed to execute the command");
    }

    if (res->getPid() == -1) { throw std::runtime_error("Failed to fork"); }
    res->is_attached = true;
    return res;
  }

  std::unique_ptr<Process> Process::fork(bool create_pty, ClosePolicy close_policy) {
    auto res = std::make_unique<Process>(0, close_policy);

    // We need to creates the pipes before forking
    if (create_pty and openpty(&res->master_ptty, &res->slave_ptty, nullptr, nullptr, nullptr)) {
      throw std::runtime_error("Failed to create pipes");
    }
    // By default, the pty has echo enabled, meaning that the user will see what he types
    // We disable this by changing the terminal settings
    termios tty_param = {};
    tcgetattr(res->master_ptty, &tty_param);
    tty_param.c_lflag &= ~ECHO;
    tcsetattr(res->master_ptty, TCSANOW, &tty_param);

    // Required before the slave pty can be used
    unlockpt(res->slave_ptty);

    res->pid = ::fork();
    return res;
  }

  Process::Status Process::getStatus() const {
    std::shared_lock<std::shared_mutex> lock(mutex);
    return status;
  }

  void Process::updateStatus(Status s) {
    std::scoped_lock<std::shared_mutex> lock(mutex);
    status = s;
  }

  bool Process::isProbeable() const {
    return isProbeableStatus(status);
  }

  bool Process::resume() {
    std::scoped_lock<std::shared_mutex> lock(mutex);
    bool res = ptrace(PTRACE_CONT, pid, nullptr, nullptr) == 0;
    if (res) { status = Status::kRunning; }
    return res;
  }

  bool Process::pause() {
    std::scoped_lock<std::shared_mutex> lock(mutex);
    bool res = ::kill(pid, SIGSTOP) == 0;
    if (res) { status = Status::kStopped; }
    return res;
  }

  bool Process::kill() {
    std::scoped_lock<std::shared_mutex> lock(mutex);
    if (status == Status::kDead) return true;
    if (::kill(pid, SIGKILL) == 0) {
      waitpid(pid, nullptr, 0);
      status = Status::kDead;
      return true;
    }
    return false;
  }

  bool Process::attach() {
    std::scoped_lock<std::shared_mutex> lock(mutex);
    bool failure = ptrace(PTRACE_ATTACH, pid, nullptr, nullptr);
    waitpid(pid, nullptr, 0);
    is_attached = not failure;
    return is_attached;
  }

  bool isProbeableStatus(Process::Status status) {
    return status != Process::Status::kRunning and status != Process::Status::kDead;
  }

}// namespace ldb
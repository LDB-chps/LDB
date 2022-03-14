#include "Process.h"
#include <csignal>
#include <fcntl.h>
#include <iostream>
#include <mutex>
#include <pty.h>
#include <sys/ptrace.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


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
        return "sigpwr: Power failure restart";
      case Signal::kSIGSYS:
        return "sigsys: Bad system call";
    }
    return "Unknown signal";
  }

  Process::Process(pid_t pid) : pid(pid) {
    if (pid != -1) status = getStatus();
    master_fd = -1;
    slave_fd = -1;
  }

  Process::~Process() {
    std::scoped_lock<std::shared_mutex> lock(mutex);
    if (pid != -1) return;

    if (status == Status::kRunning or status == Status::kStopped) kill();
    // Close all pipes if they are still open
    // No need to check for failure since those pipes cannot be used after we return from this
    // destructor
    close(master_fd);
    close(slave_fd);
  }

  // Mutexes are non-copyable, so we cannot use the default move operators
  Process::Process(Process&& other) noexcept : master_fd(-1), slave_fd(-1) {
    *this = std::move(other);
  }

  Process& Process::operator=(Process&& other) noexcept {
    std::scoped_lock<std::shared_mutex> lock(mutex);
    pid = other.pid;
    status = other.status;
    master_fd = other.master_fd;
    slave_fd = other.slave_fd;

    other.pid = -1;
    other.status = Status::kUnknown;
    other.master_fd = -1;
    other.slave_fd = -1;
    return *this;
  }


  std::unique_ptr<Process> Process::fromCommand(const std::string& command,
                                                const std::vector<std::string>& args,
                                                bool pipe_output) {
    // In the case where we want to pipe the output, we must creates the pipes before forking
    // Thus we temporarily init the process with a -1 pid
    auto res = Process::fork(pipe_output);

    if (res->getPid() == 0) {
      ptrace(PTRACE_TRACEME, 0, nullptr, nullptr);

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

      if (pipe_output) {
        // Redirect both stdout and stderr to the pipe
        dup2(res->slave_fd, STDOUT_FILENO);
        dup2(res->slave_fd, STDERR_FILENO);

        // Redirect the pipe to stdin
        dup2(res->slave_fd, STDIN_FILENO);
      }

      // Unfortunately, execvp does not respect constness
      // So we have to do a const_cast...
      // We really should manually copy the arguments properly,
      // But I don't want to spend too much time on that
      execvp(command.c_str(), const_cast<char* const*>(argv_c.data()));
      // Should only happen if execvp fails
      exit(1);
    }

    if (res->getPid() == -1) { throw std::runtime_error("Failed to fork"); }
    return res;
  }

  std::unique_ptr<Process> Process::fork(bool create_pty) {
    auto res = std::make_unique<Process>(0);

    // We need to creates the pipes before forking
    if (create_pty and openpty(&res->master_fd, &res->slave_fd, nullptr, nullptr, nullptr)) {

      throw std::runtime_error("Failed to create pipes");
    }
    // By default, the pty has echo enabled, meaning that the user will see what he types
    // We disable this by changing the terminal settings
    termios tty_param = {};
    tcgetattr(res->master_fd, &tty_param);
    tty_param.c_lflag &= ~ECHO;
    tcsetattr(res->master_fd, TCSANOW, &tty_param);

    // Required before the slave pty can be used
    unlockpt(res->slave_fd);


    res->pid = ::fork();
    // Update the status after changing its pid
    res->getStatus();
    return res;
  }

  Process::Status Process::getStatus() {
    std::shared_lock<std::shared_mutex> lock(mutex);
    return status;
  }

  Process::Status Process::waitNextEvent() {
    int s;
    int res = waitpid(pid, &s, 0);

    // Lock the mutex before accessing the status
    std::scoped_lock<std::shared_mutex> lock(mutex);
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
    // Handle signals that can be caught
    if (WIFSTOPPED(s)) {
      // ptrace stops when receiving any signal fatal or not.
      // Instead of checking for fatal signals, we check for the stop signal and consider other
      // signals as fatal
      last_signal = static_cast<Signal>(WSTOPSIG(s));
      if (WSTOPSIG(s) == SIGSTOP) { return status = Status::kStopped; }
      return status = Status::kKilled;
    }
    // Handle signals that can't be caught
    if (WIFSIGNALED(s)) { return status = Status::kKilled; }
    return Status::kRunning;
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
    if (::kill(pid, SIGTERM) == 0) {
      waitpid(pid, nullptr, 0);
      status = Status::kDead;
      return true;
    }
    return false;
  }

  bool Process::attach() {
    std::scoped_lock<std::shared_mutex> lock(mutex);
    bool failure = ptrace(PTRACE_ATTACH, pid, nullptr, nullptr);
    is_attached = not failure;
    return is_attached;
  }

  bool isProbeableStatus(Process::Status status) {
    return status == Process::Status::kStopped or status == Process::Status::kKilled;
  }

}// namespace ldb
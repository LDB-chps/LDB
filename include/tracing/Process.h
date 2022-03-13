#pragma once

#include <memory>
#include <shared_mutex>
#include <vector>

namespace ldb {

  enum class Signal {
    kUnknown,
    kSIGHUP,
    kSIGINT,
    kSIGQUIT,
    kSIGILL,
    kSIGTRAP,
    kSIGABRT,
    kSIGBUS,
    kSIGFPE,
    kSIGKILL,
    kSIGUSR1,
    kSIGSEGV,
    kSIGUSR2,
    kSIGPIPE,
    kSIGALRM,
    kSIGTERM,
    kSIGSTKFLT,
    kSIGCHLD,
    kSIGCONT,
    kSIGSTOP,
    kSIGTSTP,
    kSIGTTIN,
    kSIGTTOU,
    kSIGURG,
    kSIGXCPU,
    kSIGXFSZ,
    kSIGVTALRM,
    kSIGPROF,
    kSIGWINCH,
    kSIGIO,
    kSIGPWR,
    kSIGSYS
  };

  std::string signalToString(Signal signal);

  /**
   * @brief Thread-safe process handle
   *
   */
  class Process {
  public:
    enum class Status { kUnknown, kRunning, kExited, kKilled, kDead, kStopped };

    /**
     * @brief Construct a new Process handle associated with the given pid. Does not start the
     * process.
     * @param pid the pid to link to
     */
    explicit Process(pid_t pid);

    /**
     * @brief Kills the process if it is alive.
     */
    ~Process();

    /**
     * @brief Process Handle should not be copyable to avoid concurrent access
     */
    Process(const Process&) = delete;
    Process& operator=(const Process&) = delete;

    /**
     * @brief Thread safe move copy. Beware that this does not guarantee that other threads won't be
     * able to acces the process after it has been moved. It is up to the user to ensure that no
     * other thread is accessing the process after it has been moved.
     */
    Process(Process&&) noexcept ;
    Process& operator=(Process&&) noexcept ;

    /**
     * @brief Launch the command with its argument in a new process and return a Process handle to
     * it. The new process will automatically call ptrace() to attach to itself to the parent
     * process.
     *
     * @param command The command to launch
     * @param args The arguments to pass to the command.
     * @param pipe_output The subprocess output and input will be redirected to dedicated pipes.
     * @return Process The process handle to the launched process.
     */
    static std::unique_ptr<Process> fromCommand(const std::string& command,
                                                const std::vector<std::string>& args,
                                                bool pipe_output = false);

    /**
     * @brief Fork a new process and return a Process handle to it.
     * @param create_pty If true, creates a pseudo terminal for the subprocess output and input.
     * @return A new process handle.
     */
    static std::unique_ptr<Process> fork(bool create_pty = false);

    /**
     * @brief Returns the current status of the process
     * This function does not block and does not update the status, only the last known status.
     * @return
     */
    Status getStatus();

    /**
     * @brief Wait for an even to happen and update the process status
     * @return The new status
     */
    Status waitNextEvent();

    /**
     * @brief Signal the process to resume execution.
     * @return True if the signal was sent successfully, false otherwise.
     */
    bool resume();

    /**
     * @brief Signal the process to pause
     * @return True if the signal was correctly sent, false otherwise
     */
    bool pause();

    /**
     * @brief Kill the process if it is running
     * @return
     */
    bool kill();

    /**
     * @brief Get the pid of the process. Does not mean the process is running
     * @return The pid of the process
     */
    pid_t getPid() const {
      return pid;
    }

    /**
     * @brief Returns true if we're attached to the process, false otherwise
     * @return
     */
    bool isAttached() const {
      return is_attached;
    }

    /**
     * @brief Attempt to attach to the process
     * @return True if the process is running and we attached to it, false otherwise
     */
    bool attach();

    /**
     * @brief If the process posses a pseudo terminal, returns the file descriptor of the master
     * @return
     */
    int getMasterFd() const {
      return master_fd;
    }

    /**
     * @brief If the process posses a pseudo terminal, returns the file descriptor of the slave
     * @return
     */
    int getSlaveFd() const {
      return slave_fd;
    }

    /**
     * @brief Returns the last signal that was raised by the process
     * @return The last signal that was raised by the process, or kUnknown if not signal was raised
     */
    Signal getLastSignal() const {
      return last_signal;
    }

  private:
    /**
     * @brief The subprocess will use those pipes for redirecting its output
     */
    int slave_fd;

    /**
     * @brief The subprocess will read its input from this pipe
     */
    int master_fd;

    pid_t pid = 0;
    Status status = Status::kUnknown;
    Signal last_signal = Signal::kUnknown;
    bool is_attached = false;
    std::shared_mutex mutex;
  };

  /**
   * @brief Returns true if the given status is ok for probing, false otherwise
   * @param status The status to check
   * @return
   */
  bool isProbeableStatus(Process::Status status);
}// namespace ldb

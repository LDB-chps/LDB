#pragma once
#include <memory>
#include <shared_mutex>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace ldb {

  enum class Signal {
    kUnknown = 0,
    kSIGHUP = 1,
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
    kSIGSYS,
    kSignalCount
  };

  std::string signalToString(Signal signal);

  /**
   * @brief Thread-safe process handle
   *
   */
  class Process {
  public:
    enum class Status { kUnknown, kRunning, kExited, kKilled, kDead, kStopped };
    enum class ClosePolicy { kKill, kDetach, kWait };

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
                                                bool pipe_output = false,
                                                ClosePolicy close_policy = ClosePolicy::kKill);

    /**
     * @brief Construct a new Process handle associated with the given pid. Does not start the
     * process.
     * @param pid the pid to link to
     */
    Process(pid_t pid, ClosePolicy close_policy = ClosePolicy::kKill);

    /**
     * @brief Apply the close policy to the process
     */
    ~Process();

    void updateStatus(Status s);

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
    Process(Process&&) noexcept;
    Process& operator=(Process&&) noexcept;

    /**
     * @brief Fork a new process and return a Process handle to it.
     * @param create_pty If true, creates a pseudo terminal for the subprocess output and input.
     * @return A new process handle.
     */
    static std::unique_ptr<Process> fork(bool create_pty = false,
                                         ClosePolicy close_policy = ClosePolicy::kKill);

    /**
     * @brief Returns the current status of the process
     * This function does not block and does not update the status, only the last known status.
     * @return
     */
    Status getStatus() const;

    /**
     * @brief Returns true if the process is in a status that is available for probing via ptrace
     *
     * To be able to be traced, a process must neither be dead nor running, in a "stopped" state
     */
    bool isProbeable() const;

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
      return master_ptty;
    }

    /**
     * @brief If the process posses a pseudo terminal, returns the file descriptor of the slave
     * @return
     */
    int getSlaveFd() const {
      return slave_ptty;
    }
    
  private:
    /**
     * @brief The subprocess will use those pipes for redirecting its output
     */
    int slave_ptty;

    /**
     * @brief The subprocess will read its input from this pipe
     */
    int master_ptty;

    pid_t pid = 0;
    Status status = Status::kUnknown;
    bool is_attached = false;

    Process::ClosePolicy close_policy;

    // The mutex should be mutable because it needs to be locked even in const methods
    mutable std::shared_mutex mutex;
  };

  /**
   * @brief Returns true if the given status is ok for probing, false otherwise
   * @param status The status to check
   * @return
   */
  bool isProbeableStatus(Process::Status status);
}// namespace ldb

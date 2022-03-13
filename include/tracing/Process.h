#pragma once

#include <memory>

namespace ldb {

  /**
   * @brief Lightweight non-copyable process handle
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

    ~Process();

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
    static std::unique_ptr<Process> fromCommand(const std::string& command, const std::string& args,
                                                bool pipe_output = false);

    /**
     * @brief Fork a new process and return a Process handle to it.
     * @param create_pipe If true, creates a pipe for the subprocess output and input.
     * @return A new process handle.
     */
    static std::unique_ptr<Process> fork(bool create_pipe = false);

    /**
     * @brief Process Handle should not be copyable to avoid concurrent access
     */
    Process(const Process&) = delete;
    Process& operator=(const Process&) = delete;

    Process(Process&&) = default;
    Process& operator=(Process&&) = default;

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
     * @brief Wait for the process to exit or for a signal to raise.
     */
    void wait();

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

    int getMasterFd() {
      return master_fd;
    }

    int getSlaveFd() {
      return slave_fd;
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
    bool is_attached = false;
  };
}// namespace ldb

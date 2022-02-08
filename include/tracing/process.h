#pragma once

#include <memory>

namespace ldb {

  /**
   * @brief Lightweight non-copyable process handle
   */
  class Process {
  public:
    /**
     * @brief Construct a new Process handle associated with the given pid. Does not start the
     * process.
     * @param pid the pid to link to
     */
    explicit Process(pid_t pid);

    /**
     * @brief Process Handle are not copyable
     */
    Process(const Process&) = delete;

    /**
     * @brief Process Handle are not copyable
     */
    Process& operator=(const Process&) = delete;

    Process(Process&&) = default;
    Process& operator=(Process&&) = default;

    /**
     * @brief Launch the command with its argument in a new process and return a Process handle to
     * it. The new process will automatically call ptrace() to attach to itself to the parent
     * process.
     *
     * @param command
     * @param args
     * @return
     */
    static Process fromCommand(const std::string& command, const std::string& args);

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
    pid_t getPid() {
      return pid;
    }

    /**
     * @brief Check if the process is running
     * @return True if the process is running, false otherwise
     */
    bool isAlive();

    /**
     * @brief Wait for the process to exit or for a signal to raise.
     */
    void wait();

  private:
    pid_t pid = 0;
  };
}// namespace ldb

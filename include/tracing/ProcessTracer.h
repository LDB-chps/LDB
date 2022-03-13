
#pragma once

#include "Process.h"
#include "RegistersSnapshot.h"
#include <filesystem>
#include <memory>
#include <shared_mutex>
#include <thread>
#include <vector>

namespace ldb {

  /**
   * @brief Frontend class that can be used to trace a process and yield useful
   * information for debugging.
   *
   * This class provides various methods to trace a process and yield useful information for
   * debugging. Note that this requires that the process is suspended, and the class will return no
   * data otherwise.
   *
   * We recommend calling isSuspended() method before trying to access any of the getter methods.
   */
  class ProcessTracer {
  public:
    /**
     * @brief launch a new process and trace it
     * @param executable the program to execute
     * @param args the args of the program
     * @return A tracer attached to the new process
     */
    static std::unique_ptr<ProcessTracer> fromCommand(const std::string& executable,
                                                      const std::vector<std::string>& args);

    ProcessTracer(Process&& process, std::string executable);

    /**
     * @brief Yield the current process registers values
     * @return
     */
    std::unique_ptr<RegistersSnapshot> getRegistersSnapshot();

    /**
     * @brief Returns the path to the executable linked to this tracer
     * @return The path to the executable linked to this tracer, or an empty string if this data is
     * unavailable
     */
    std::string getExecutable();

    Process& getProcess() {
      return process;
    }

    pid_t getPid() const {
      return process.getPid();
    }

    Signal getLastSignal() const {
      return process.getLastSignal();
    }

    /**
     * @brief Returns the current file the process is in
     * @return A path to the source file, or an empty path if this data is unavailable
     */
    std::string getCurrentFile();

    /**
     * @brief Returns the current line the process is in
     * @return The current line the process is in, or -1 if this data is unavailable
     */
    long getCurrentLineNumber();

    /**
     * @brief Returns the current function the process is in
     * @return Returns the current function name, or an empty string if this data is unavailable
     */
    std::string getCurrentFunctionName();

    /**
     * @brief The process this tracer is attached to has its output redirected to a file
     * This functions returns the file descriptor of this file. This can be used for reading the
     * output to a QtWindow
     * @return
     */
    int getSlaveFd() {
      return process.getSlaveFd();
    }

    /**
     * @brief The process this tracer is attached to has its output redirected to a file
     * This functions returns the file descriptor of this file. This can be used for reading the
     * output to a QtWindow
     * @return
     */
    int getMasterFd() {
      return process.getMasterFd();
    }

    /**
     * @brief Block until the process receives a signal or terminates
     * @return The status of the process after the wait
     */
    Process::Status waitNextEvent();

    /**
     * @brief Returns a vector containing the full stacktrace of the process
     * @return A vector containing the full stacktrace of the process, or an empty vector if this
     * data is unavailable
     */
    // std::unique_ptr<StackTrace> getStackTrace();

    Process::Status getProcessStatus() {
      return process.getStatus();
    }

  private:
    /** A thread is created to handle the process
     * Therefore, a lock is used to avoid concurency
     */
    std::shared_mutex main_mutex;

    Process process;

    std::string executable_path;

    // SymbolTable symbols;
  };

}// namespace ldb

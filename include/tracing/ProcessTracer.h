
#pragma once

#include "DebugInfo.h"
#include "ELFParser.h"
#include "Process.h"
#include "RegistersSnapshot.h"
#include "SignalHandler.h"
#include "StackTrace.h"
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

    /**
     * @brief Builds a new tracer from an existing process
     * @param debug_info The debug infos read from the elf file
     * @param process The process to trace. Must be different than nullptr, and already traced.
     * @param executable The executable of the process.
     * @param args The arguments used to launch the process.
     */
    ProcessTracer(std::unique_ptr<Process>&& process, std::string executable,
                  std::vector<std::string> args);

    /**
     * @brief Returns the path to the executable linked to this tracer
     * @return The path to the executable linked to this tracer, or an empty string if this data is
     * unavailable
     */
    const std::string& getExecutable();

    bool restart();

    const Process& getProcess() const {
      return *process;
    }

    void resume() {
      process->resume();
    }

    void pause() {
      process->pause();
    }

    void abort() {
      process->kill();
    }

    /**
     * @brief The process this tracer is attached to has its output redirected to a file
     * This functions returns the file descriptor of this file. This can be used for reading the
     * output to a QtWindow
     * @return
     */
    int getSlavePtty() {
      return process->getSlaveFd();
    }

    /**
     * @brief The process this tracer is attached to has its output redirected to a file
     * This functions returns the file descriptor of this file. This can be used for reading the
     * output to a QtWindow
     * @return
     */
    int getMasterPtty() {
      return process->getMasterFd();
    }

    /**
     * @brief Returns a vector containing the full stacktrace of the process
     * @return A vector containing the full stacktrace of the process, or an empty vector if this
     * data is unavailable
     */
    std::unique_ptr<StackTrace> getStackTrace();

    /**
     * @brief Yield the current process registers values
     * @return
     */
    std::unique_ptr<RegistersSnapshot> getRegistersSnapshot() const;

    const DebugInfo* getDebugInfo() {
      if (not debug_info and isProbeableStatus(process->getStatus())) {
        debug_info = readDebugInfo(executable_path, *process);
      }
      return debug_info.get();
    }

    template<class Sighandler>
    Sighandler* makeSignalHandler() {
      auto tmp = std::make_unique<Sighandler>(process.get());
      // Get the ret ptr before type casting to parent class
      auto res = tmp.get();
      signal_handler = std::move(tmp);
      return res;
    }

    SignalHandler* getSignalHandler() {
      return signal_handler.get();
    }

  private:
    std::unique_ptr<Process> process;

    std::string executable_path;
    std::vector<std::string> arguments;

    std::unique_ptr<const DebugInfo> debug_info;

    std::unique_ptr<SignalHandler> signal_handler;
  };

}// namespace ldb

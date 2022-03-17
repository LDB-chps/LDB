
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
   * @brief Main class that handles tracing of a process.
   *
   * Provides utility from starting and pausing a process, attaching a signal handler, and getting
   * information such as a stacktrace and register snapshots.
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

    /**
     * @brief Restart the process using the same arguments used to launch it in the first place
     * @return True if the process was restarted, false otherwise
     */
    bool restart();

    const Process& getProcess() const {
      return *process;
    }

    void resume() {
      process->resume();
      signal_handler->unmute();
    }

    void pause() {
      process->pause();
      signal_handler->mute();
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
     * @brief Returns the current stacktrace of the process if it is stopped
     * @return A unique_ptr to as StackTrace object, or nullptr if the process is not stopped or an
     * error occurred
     */
    std::unique_ptr<StackTrace> getStackTrace();

    /**
     * @brief Yield the current process registers values
     * @return A unique_ptr to a RegistersSnapshot object, or nullptr if an error occurred or the
     * process is not stopped
     */
    std::unique_ptr<RegistersSnapshot> getRegistersSnapshot() const;

    /**
     * @brief Load and parse the debug information of the process, and cache it for future requests
     * If this operation fails, the debug information is not cached and the next call to this
     * function will NOT try to load it again
     *
     * @return A pointer to the loaded debug information, or nullptr if an error occurred or no
     * debug symbols are available
     */
    const DebugInfo* getDebugInfo() {
      if (not debug_info and not failed_read_debug_info and
          isProbeableStatus(process->getStatus())) {
        debug_info = readDebugInfo(executable_path, *process);
        failed_read_debug_info = debug_info != nullptr;
      }
      return debug_info.get();
    }

    /**
     * @brief Adds a signal handler to the process.
     * @tparam Sighandler The type of the signal handler to add.
     * @return A pointer to the added signal handler, or nullptr if an error occurred
     */
    template<class Sighandler>
    Sighandler* makeSignalHandler() {
      auto tmp = std::make_unique<Sighandler>(process.get());
      // Get the res ptr before type casting to parent class
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

    // When the user first tries to access the debug_info, we need to read it.
    // It cannot be parsed beforehand since we may parse it before the process exits the
    // execve trap.
    // This flag only serves to indicate that we tried to read the debug_info, and failed
    // to prevent future attempts
    bool failed_read_debug_info = false;
  };

}// namespace ldb

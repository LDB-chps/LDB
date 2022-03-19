
#pragma once

#include "BreakPointHandler.h"
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
     * @brief Start a new tracee
     * @param command
     * @param args
     */
    ProcessTracer(const std::string& command, const std::vector<std::string>& args);

    /**
     * @brief Returns the path to the executable linked to this tracer
     * @return The path to the executable linked to this tracer, or an empty string if this data is
     * unavailable
     */
    const std::string& getExecutable();

    const Process& getProcess() const {
      return *process;
    }


    /**
     * @brief Restart the process using the same arguments used to launch it in the first place
     * @return True if the process was restarted, false otherwise
     */
    bool restart();

    void resume() {
      if (breakpoint_handler->isAtBreakpoint()) {
        signal_handler->mute();
        breakpoint_handler->resetBreakpoint();
        signal_handler->unmute();
      }

      process->resume();
    }

    void singlestep() {
      if (process->getStatus() != Process::Status::kStopped) return;
      if (breakpoint_handler->isAtBreakpoint()) {
        signal_handler->mute();
        breakpoint_handler->resetBreakpoint();
        signal_handler->unmute();
      }

      // By default, the breakpoint handler jump to the next instruction when restoring a breakpoint
      ptrace(PTRACE_SINGLESTEP, process->getPid(), nullptr, nullptr);
    }

    void pause() {
      process->pause();
    }

    void abort() {
      process->kill();
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
      return debug_info.get();
    }

    const SymbolTable* getSymbolTable() {
      if (not debug_info) return nullptr;
      return debug_info->getSymbolTable();
    }

    /**
     * @brief Returns the current state of the process
     * @return The current state of the process
     */
    BreakPointHandler* getBreakPointHandler() {
      return breakpoint_handler.get();
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
    bool readSymbols();

    std::unique_ptr<Process> process;

    std::string executable_path;
    std::vector<std::string> arguments;

    std::unique_ptr<const DebugInfo> debug_info;

    std::unique_ptr<SignalHandler> signal_handler;

    std::unique_ptr<BreakPointHandler> breakpoint_handler;
  };

}// namespace ldb

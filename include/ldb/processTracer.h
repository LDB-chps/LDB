
#pragma once

#include "process.h"
#include <thread>
#include <mutex>

namespace ldb {

  /**
   * @brief ProcessTracer is a class that can be used to trace a process and yield useful information for debugging.
   *
   * This class provides various methods to trace a process and yield useful information for debugging. Note that this
   * requires that the process is suspended, and the class will return no data otherwise.
   *
   * We recommend calling isSuspended() method before trying to access any of the getter methods.
   */
  class processTracer {
  public:

    /**
     * @brief launch a new process and trace it
     * @param command the program to execute
     * @param args the args of the program
     * @return A tracer attached to the new process
     */
    static std::unique_ptr<ProcessTracer>(const std::string &command, std::vector<std::string> &args);

    /**
     * @brief Yield the current process registers values
     * @return
     */
    std::vector<RegisterValue> getRegistersValues();

    /**
     * @brief Yield the current process global variable values
     * @return
     */
    std::vector<VariableValue> getGlobalVariablesValues();

    // TODO: Implement those if time permits
    // std::vector<VariableValue> getLocalVariableValues();
    // std::vector<VariableValue> getArgumentVariableValues();

    /**
     * @brief Returns the path to the executable linked to this tracer
     * @return The path to the executable linked to this tracer, or an empty string if this data is unavailable
     */
    std::path getExecutablePath();

    /**
     * @brief Returns the current file the process is in
     * @return A path to the source file, or an empty path if this data is unavailable
     */
    std::path getCurrentFilePath();

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
     * @brief Returns a vector containing the full stacktrace of the process
     * @return A vector containing the full stacktrace of the process, or an empty vector if this data is unavailable
     */
    std::vector<std::string> getStackTrace();

  private:
    /** A thread is created to handle the process
     * Therefore, a lock is used to avoid concurency
     */
    std::mutex main_mutex;
    std::unique_ptr<Process> process;

    // TODO: Implement a symbol table for address to name translation and vice-versa
    // The symbol table should allow one to find the name of a function and its associated file and line number.
    // It should also allow one to search for a particular symbol type (i.e function, variable, global variable, etc)
    // std::unique_ptr<SymbolTable>

    // True if the process is running
    bool is_running;

    // True if the process is suspended
    bool is_suspended;

    // True if the process is in a good state
    // (i.e the program is running or suspended, and not segfaulted)
    bool is_good;
  };

}



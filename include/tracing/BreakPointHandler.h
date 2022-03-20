#pragma once
#include <elf.h>
#include <iostream>
#include <optional>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/wait.h>
#include <unistd.h>
#include "SymbolTable.h"
#include "Symbol.h"
#include "BreakPointTable.h"

namespace ldb {

  /**
   * @brief Handle of breakpoints table to give funtions to manipulate it 
   * 
   */
  class BreakPointHandler {
  public:
    BreakPointHandler(const pid_t pid);

    const BreakPointTable& getBreakPoints() const {
      return breakPoints;
    }

    /**
     * @brief Adding a break point
     * 
     * @param sym Symbol to add
     */
    void add(const Symbol& sym);

    /**
     * @brief Removing a break point
     * 
     * @param sym Symbol to remove
     */
    void remove(const Symbol& sym);

    /**
     * @brief Removed all existing break point
     * 
     */
    void removeAll();

    /**
     * @brief Save name of existing break point
     * 
     * @param symbols table of all symbols of child child process
     * @return const std::vector<std::string> list of name of break point name
     */
    const std::vector<std::string> saveBreakpoints(const SymbolTable& symbols);

    /**
     * @brief Update the pid of child process in this class
     * 
     * @param pid new pid
     */
    void resetPid(const pid_t pid);

    /**
     * @brief Rebuild the breakPointTable
     * 
     * @param symbols Table of all symbols of child child process
     * @param old list of break point name to add
     */
    void refreshBreakPoint(const SymbolTable& symbols, const std::vector<std::string>& old);

    bool isAtBreakpoint() const;
    bool isBreakPoint(Elf64_Addr addr) const;

    /**
     * @brief Do the process of remove break point, 
     * step on instruction and submit the break point
     * 
     * @return true if all is well
     * @return false else
     */
    bool resetBreakpoint();

  private:
    /**
     * @brief Restore the good instruction of breakpoint
     * 
     * @param addr address of break point
     */
    void restoreInstruction(Elf64_Addr addr);

    /**
     * @brief Execute one instruction
     * 
     * @param addr address of break point
     */
    void executeInstruction(Elf64_Addr addr);

    /**
     * @brief Submit the breakpoint with bad instruction
     * 
     * @param addr address of break point
     */
    void restoreBreakpoint(Elf64_Addr addr);


    pid_t pid;
    BreakPointTable breakPoints;
    std::optional<Elf64_Addr> currentAddr;
  };

}// namespace ldb
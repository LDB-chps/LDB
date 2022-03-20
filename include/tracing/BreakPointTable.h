#pragma once
#include <elf.h>
#include <iostream>
#include <map>
#include <sys/ptrace.h>
#include "SymbolTable.h"


namespace ldb {

  /**
   * @brief Object contain list of all break points
   * 
   */
  class BreakPointTable {
    friend std::ostream& operator<<(std::ostream& os, const BreakPointTable& table);

  public:
    BreakPointTable() = default;

    BreakPointTable(const BreakPointTable& other) = delete;
    BreakPointTable& operator=(const BreakPointTable& other) = delete;

    const std::map<Elf64_Addr, unsigned long>& getBreakPoints() const {
      return breakPoints;
    }

    /**
     * @brief Adding a break point
     *
     * @param pid pid of process
     * @param addr address of break point
     */
    void add(const pid_t pid, const Elf64_Addr addr);

    /**
     * @brief Removing a break point
     *
     * @param pid pid process
     * @param addr address of break point
     */
    void remove(const pid_t pid, const Elf64_Addr addr);
    void removeAll();

    void refresh(const SymbolTable& symbols, const std::vector<std::string>& old);

    /**
     * @brief Check il the address points one a break point in the map
     *
     * @param addr address of break point
     * @return true if is a break point
     * @return false else
     */
    const bool isBreakPoint(const Elf64_Addr addr) const;

  private:
    std::map<Elf64_Addr, unsigned long> breakPoints;
  };

  std::ostream& operator<<(std::ostream& os, const BreakPointTable& info);

}// namespace ldb
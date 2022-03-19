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

  class BreakPointHandler {
  public:
    BreakPointHandler(const pid_t pid);

    const BreakPointTable& getBreakPoints() const {
      return breakPoints;
    }

    void add(const Symbol& sym);
    void remove(const Symbol& sym);
    void removeAll();


    const std::vector<std::string> saveBreakpoints(const SymbolTable& symbols);
    void refreshPid(const pid_t p);
    void refreshBreakPoint(const SymbolTable& symbols, const std::vector<std::string>& old);

    bool isAtBreakpoint() const;
    bool isBreakPoint(Elf64_Addr addr) const;

    bool resetBreakpoint();

  private:
    void restoreInstruction(Elf64_Addr addr);
    void executeInstruction(Elf64_Addr addr);
    void restoreBreakpoint(Elf64_Addr addr);


    pid_t pid;
    BreakPointTable breakPoints;
    std::optional<Elf64_Addr> currentAddr;
  };

}// namespace ldb
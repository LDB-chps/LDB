#include <elf.h>
#include <iostream>
#include <optional>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/wait.h>
#include <unistd.h>
#include "Symbol.h"
#include "BreakPointTable.h"

namespace ldb {

  class BreakPointHandler {
  public:
    BreakPointHandler(const pid_t pid);

    const BreakPointTable& getBreakPoints() const {
      return breakpoints;
    }

    void add(const Symbol& sym);

    void remove(const Symbol& sym);

    bool isAtBreakpoint() const;
    bool isBreakPoint(Elf64_Addr addr) const;


    bool resetBreakpoint();

  private:
    void restoreInstruction(Elf64_Addr addr);
    void executeInstruction(Elf64_Addr addr);
    void restoreBreakpoint(Elf64_Addr addr);


    const pid_t pid;
    BreakPointTable breakpoints;
    std::optional<Elf64_Addr> current_addr;
  };

}// namespace ldb
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
      return breakPoints;
    }

    void add(const Symbol& sym);

    void remove(const Symbol& sym);

    const bool isBreakPoint(const Elf64_Addr addr) const;

    void restoreInstruction(const Elf64_Addr addr);

    void executeBpInstruction(const Elf64_Addr addr);

    void submitBreakPoint(const Elf64_Addr addr);

  public:
    const pid_t pid;
    BreakPointTable breakPoints;
    std::optional<Elf64_Addr> currentAddr;
  };

}// namespace ldb
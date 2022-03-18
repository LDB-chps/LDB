#include "BreakPointHandler.h"

namespace ldb {

  BreakPointHandler::BreakPointHandler(const pid_t pid) : pid(pid){};

  void BreakPointHandler::add(const Symbol& sym) {
    breakPoints.add(pid, sym.getAddress());
  }

  void BreakPointHandler::remove(const Symbol& sym) {
    breakPoints.remove(pid, sym.getAddress());
  }

  const bool BreakPointHandler::isBreakPoint(const Elf64_Addr addr) const {
    return breakPoints.isBreakPoint(addr);
  }

  void BreakPointHandler::restoreInstruction(const Elf64_Addr addr) {
    if (currentAddr) throw std::runtime_error("Old breakpoint not submitted");

    auto it = breakPoints.getBreakPoints().find(addr);
    if (it == breakPoints.getBreakPoints().end()) throw std::runtime_error("Breakpoint not found");

    currentAddr = addr;

    ptrace(PTRACE_POKETEXT, pid, addr, it->second);
    ptrace(PTRACE_POKEUSER, pid, 8 * RIP, addr);
  }

  void BreakPointHandler::executeBpInstruction(const Elf64_Addr addr) {
    if (!currentAddr && currentAddr != addr) throw std::runtime_error("No breakpoint to execute");

    ptrace(PTRACE_SINGLESTEP, pid, NULL, NULL);
  
    waitpid(pid, NULL, 0);
  }

  void BreakPointHandler::submitBreakPoint(const Elf64_Addr addr) {
    if (!currentAddr && currentAddr.value() != addr)
      throw std::runtime_error("No breakpoint to submit");

    auto it = breakPoints.getBreakPoints().find(currentAddr.value());
    if (it == breakPoints.getBreakPoints().end()) throw std::runtime_error("Breakpoint not found");

    const unsigned long bpInstruction = (it->second & 0xFFFFFFFFFFFFFF00) | 0x00000000000000CC;
    ptrace(PTRACE_POKETEXT, pid, addr, bpInstruction);
    currentAddr = std::nullopt;
  }

}// namespace ldb
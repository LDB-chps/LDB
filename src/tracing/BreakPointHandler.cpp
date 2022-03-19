#include "BreakPointHandler.h"

namespace ldb {

  BreakPointHandler::BreakPointHandler(const pid_t pid) : pid(pid){};

  void BreakPointHandler::add(const Symbol& sym) {
    breakpoints.add(pid, sym.getAddress());
  }

  void BreakPointHandler::remove(const Symbol& sym) {
    breakpoints.remove(pid, sym.getAddress());
  }

  bool BreakPointHandler::isBreakPoint(const Elf64_Addr addr) const {
    return breakpoints.isBreakPoint(addr);
  }

  bool BreakPointHandler::isAtBreakpoint() const {
    unsigned long rip = ptrace(PTRACE_PEEKUSER, pid, 8 * RIP, NULL);
    if (rip < 0) return false;
    return isBreakPoint(rip - 1);
  }

  bool BreakPointHandler::resetBreakpoint() {
    if (not isAtBreakpoint()) return false;
    Elf64_Addr rip = ptrace(PTRACE_PEEKUSER, pid, 8 * RIP, NULL);
    if (rip <= 1) return false;

    restoreInstruction(rip - 1);
    executeInstruction(rip - 1);
    restoreBreakpoint(rip - 1);
    return true;
  }

  void BreakPointHandler::restoreInstruction(const Elf64_Addr addr) {
    if (current_addr) throw std::runtime_error("Old breakpoint not submitted");

    auto it = breakpoints.getBreakPoints().find(addr);
    if (it == breakpoints.getBreakPoints().end()) throw std::runtime_error("Breakpoint not found");

    current_addr = addr;

    ptrace(PTRACE_POKETEXT, pid, addr, it->second);
    ptrace(PTRACE_POKEUSER, pid, 8 * RIP, addr);
  }

  void BreakPointHandler::executeInstruction(Elf64_Addr addr) {
    if (not current_addr or current_addr != addr)
      throw std::runtime_error("No breakpoint to execute");

    ptrace(PTRACE_SINGLESTEP, pid, NULL, NULL);

    waitpid(pid, NULL, 0);
  }

  void BreakPointHandler::restoreBreakpoint(Elf64_Addr addr) {
    if (not current_addr or current_addr.value() != addr)
      throw std::runtime_error("No breakpoint to submit");

    auto it = breakpoints.getBreakPoints().find(current_addr.value());
    if (it == breakpoints.getBreakPoints().end()) throw std::runtime_error("Breakpoint not found");

    const unsigned long bpInstruction = (it->second & 0xFFFFFFFFFFFFFF00) | 0x00000000000000CC;
    ptrace(PTRACE_POKETEXT, pid, addr, bpInstruction);
    current_addr = std::nullopt;
  }

}// namespace ldb
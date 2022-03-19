#include "BreakPointHandler.h"

namespace ldb {

  BreakPointHandler::BreakPointHandler(const pid_t pid) : pid(pid){};

  void BreakPointHandler::add(const Symbol& sym) {
    breakPoints.add(pid, sym.getAddress());
  }

  void BreakPointHandler::remove(const Symbol& sym) {
    breakPoints.remove(pid, sym.getAddress());
  }

  void BreakPointHandler::removeAll() {
    breakPoints.removeAll();
  }

  const std::vector<std::string> BreakPointHandler::saveBreakpoints(const SymbolTable& symbols) {
    std::vector<std::string> res;
    const auto& bps = breakPoints.getBreakPoints();
    for (auto it = bps.cbegin(); it != bps.end(); ++it) {
      res.push_back(symbols.findInTable(it->first).first->getName());
    }
    return res;
  }

  void BreakPointHandler::refreshPid(const pid_t p) {
    pid = p;
  }

  void BreakPointHandler::refreshBreakPoint(const SymbolTable& symbols,
                                            const std::vector<std::string>& old) {
    this->pid = pid;

    breakPoints.removeAll();

    for (auto& i : old) { add(*symbols[i]); }
  }

  bool BreakPointHandler::isBreakPoint(const Elf64_Addr addr) const {
    return breakPoints.isBreakPoint(addr);
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
    if (currentAddr) throw std::runtime_error("Old breakpoint not submitted");

    auto it = breakPoints.getBreakPoints().find(addr);
    if (it == breakPoints.getBreakPoints().end()) throw std::runtime_error("Breakpoint not found");

    currentAddr = addr;

    ptrace(PTRACE_POKETEXT, pid, addr, it->second);
    ptrace(PTRACE_POKEUSER, pid, 8 * RIP, addr);
  }

  void BreakPointHandler::executeInstruction(Elf64_Addr addr) {
    if (not currentAddr or currentAddr != addr)
      throw std::runtime_error("No breakpoint to execute");

    ptrace(PTRACE_SINGLESTEP, pid, NULL, NULL);

    waitpid(pid, NULL, 0);
  }

  void BreakPointHandler::restoreBreakpoint(Elf64_Addr addr) {
    if (not currentAddr or currentAddr.value() != addr)
      throw std::runtime_error("No breakpoint to submit");

    auto it = breakPoints.getBreakPoints().find(currentAddr.value());
    if (it == breakPoints.getBreakPoints().end()) throw std::runtime_error("Breakpoint not found");

    const unsigned long bpInstruction = (it->second & 0xFFFFFFFFFFFFFF00) | 0x00000000000000CC;
    ptrace(PTRACE_POKETEXT, pid, addr, bpInstruction);
    currentAddr = std::nullopt;
  }

}// namespace ldb
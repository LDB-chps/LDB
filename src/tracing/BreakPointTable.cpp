#include "BreakPointTable.h"


namespace ldb {

  void BreakPointTable::add(const pid_t pid, const Elf64_Addr addr) {
    const unsigned long instruction = ptrace(PTRACE_PEEKTEXT, pid, addr, NULL);
    static const unsigned long bpInstruction =
            (instruction & 0xFFFFFFFFFFFFFF00) | 0x00000000000000CC;
    ptrace(PTRACE_POKETEXT, pid, addr, bpInstruction);
    breakPoints.emplace(addr, instruction);
  }

  void BreakPointTable::remove(const pid_t pid, const Elf64_Addr addr) {
    auto it = breakPoints.find(addr);
    if (it == breakPoints.end()) throw std::runtime_error("Breakpoint not found");
    ptrace(PTRACE_POKETEXT, pid, addr, it->second);
    breakPoints.erase(it);
  }

  void BreakPointTable::removeAll() {
    breakPoints.clear();
  }


  const bool BreakPointTable::isBreakPoint(const Elf64_Addr addr) const {
    return breakPoints.find(addr) != breakPoints.end();
  }


  std::ostream& operator<<(std::ostream& os, const BreakPointTable& info) {
    for (auto it = info.breakPoints.begin(); it != info.breakPoints.end(); it++)
      os << "[" << std::hex << it->first << ": " << it->second << "]" << std::endl;
    return os;
  }

}// namespace ldb
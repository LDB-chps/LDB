#include <elf.h>
#include <iostream>
#include <map>
#include <sys/ptrace.h>


namespace ldb {

  class BreakPointTable {
    friend std::ostream& operator<<(std::ostream& os, const BreakPointTable& table);

  public:
    BreakPointTable() = default;

    BreakPointTable(const BreakPointTable& other) = delete;
    BreakPointTable& operator=(const BreakPointTable& other) = delete;

    const std::map<Elf64_Addr, unsigned long> getBreakPoints() const {
      return breakPoints;
    }

    void add(const pid_t pid, const Elf64_Addr addr);

    void remove(const pid_t pid, const Elf64_Addr addr);

    const bool isBreakPoint(const Elf64_Addr addr) const;

  private:
    std::map<Elf64_Addr, unsigned long> breakPoints;
  };


  std::ostream& operator<<(std::ostream& os, const BreakPointTable& info);

}// namespace ldb
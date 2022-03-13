#include "RegistersSnapshot.h"
#include <sys/ptrace.h>
#include <sys/user.h>
#include <unistd.h>

namespace ldb {

  std::vector<RegisterValue> buildRegisterValues(const user_regs_struct& regs) {
    std::vector<RegisterValue> res;
    // Yes, this is ugly
    res.emplace_back("rax", regs.rax);
    res.emplace_back("r15", regs.r15);
    res.emplace_back("r14", regs.r14);
    res.emplace_back("r13", regs.r13);
    res.emplace_back("r12", regs.r12);
    res.emplace_back("rbp", regs.rbp);
    res.emplace_back("rbx", regs.rbx);
    res.emplace_back("r11", regs.r11);
    res.emplace_back("r10", regs.r10);
    res.emplace_back("r9", regs.r9);
    res.emplace_back("r8", regs.r8);
    res.emplace_back("rax", regs.rax);
    res.emplace_back("rcx", regs.rcx);
    res.emplace_back("rdx", regs.rdx);
    res.emplace_back("rsi", regs.rsi);
    res.emplace_back("rdi", regs.rdi);
    res.emplace_back("orig_rax", regs.orig_rax);
    res.emplace_back("rip", regs.rip);
    res.emplace_back("cs", regs.cs);
    res.emplace_back("eflags", regs.eflags);
    res.emplace_back("rsp", regs.rsp);
    res.emplace_back("ss", regs.ss);
    res.emplace_back("fs_base", regs.fs_base);
    res.emplace_back("gs_base", regs.gs_base);
    res.emplace_back("ds", regs.ds);
    res.emplace_back("es", regs.es);
    res.emplace_back("fs", regs.fs);
    res.emplace_back("gs", regs.gs);

    return res;
  }

  RegistersSnapshot::RegistersSnapshot(Process& process) {
    // We need the process to be suspended to get the registers.
    if (not isProbeableStatus(process.getStatus())) { return; }
    std::cout << "Getting registers" << std::endl;
    user_regs_struct regs{};

    if (ptrace(PTRACE_GETREGS, process.getPid(), nullptr, &regs) == -1)
      throw std::runtime_error("Failed to get registers of process " +
                               std::to_string(process.getPid()));

    registers = buildRegisterValues(regs);
  }

}// namespace ldb
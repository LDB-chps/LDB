#include "ProcessTracer.h"

#include "RegistersSnapshot.h"
#include <tscl.hpp>
#include <utility>

namespace ldb {

  std::unique_ptr<ProcessTracer> ProcessTracer::fromCommand(const std::string& command,
                                                            const std::vector<std::string>& args) {
    auto process = Process::fromCommand(command, args, true);
    if (not process) { return nullptr; }

    // Read the static symbol table
    ELFFile elf(command);
    const auto symbolTable = elf.getDebugInfo()->getSymbolTable();
    Symbol* _start_symbol = symbolTable->operator[]("_start");

    waitpid(process->getPid(), nullptr, 0);

    // Adding the _start function as break point
    BreakPointHandler bph(process->getPid());
    bph.add(*_start_symbol);

    // The process automatically stops at launch when it is traced
    // We resume it
    process->resume();

    waitpid(process->getPid(), nullptr, 0);

    unsigned long rip = ptrace(PTRACE_PEEKUSER, process->getPid(), 8 * RIP, NULL);
    if (not bph.isBreakPoint(rip - 1)) { throw std::runtime_error("Not a breakpoint"); }

    bph.restoreInstruction(rip - 1);
    bph.executeBpInstruction(rip - 1);
    bph.submitBreakPoint(rip - 1);

    elf.parseDynamicSymbols(*process);

    process->updateStatus(Process::Status::kStopped);

    return std::make_unique<ProcessTracer>(std::move(process), command, args, elf.yieldDebugInfo());
  }

  ProcessTracer::ProcessTracer(std::unique_ptr<Process>&& process, std::string executable,
                               std::vector<std::string> args,
                               std::unique_ptr<DebugInfo>&& debugInfo)
      : process(std::move(process)), executable_path(std::move(executable)),
        arguments(std::move(args)), debug_info(std::move(debugInfo)) {

    if (not this->process) throw std::runtime_error("ProcessTracer: cannot trace a null process");
  }

  bool ProcessTracer::restart() {
    process = Process::fromCommand(executable_path, arguments, true);
    if (not process) throw std::runtime_error("ProcessTracer: failed to reset the process");
    waitpid(process->getPid(), nullptr, 0);
    process->updateStatus(Process::Status::kStopped);
    signal_handler->reset(process.get());

    return true;
  }

  std::unique_ptr<RegistersSnapshot> ProcessTracer::getRegistersSnapshot() const {
    if (not isProbeableStatus(process->getStatus())) { return {}; }

    return std::make_unique<RegistersSnapshot>(*process);
  }

  const std::string& ProcessTracer::getExecutable() {
    // The executable_path is immutable, so we can safely return a copy without acquiring the mutex
    return executable_path;
  }

  std::unique_ptr<StackTrace> ProcessTracer::getStackTrace() {
    return std::make_unique<StackTrace>(*this);
  }

}// namespace ldb

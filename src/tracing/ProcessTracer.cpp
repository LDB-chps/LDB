#include "ProcessTracer.h"

#include "RegistersSnapshot.h"

namespace ldb {

  ProcessTracer::ProcessTracer(const std::string& command, const std::vector<std::string>& args)
      : executable_path(command), arguments(args) {

    process = Process::fromCommand(command, args, true);
    if (not process) throw std::runtime_error("Failed to start process");

    // Wait until we reach the entry point
    waitpid(process->getPid(), nullptr, 0);
    process->updateStatus(Process::Status::kStopped);
    breakpoint_handler = std::make_unique<BreakPointHandler>(this->process->getPid());
    readSymbols();
  }

  bool ProcessTracer::restart() {
    process = Process::fromCommand(executable_path, arguments, true);
    if (not process) {
      signal_handler->reset(nullptr);
      debug_info = nullptr;
      breakpoint_handler = nullptr;
      throw std::runtime_error("ProcessTracer: failed to reset the process");
    }
    waitpid(process->getPid(), nullptr, 0);
    process->updateStatus(Process::Status::kStopped);

    // We save breakpoint for the next execution like dynamic libs can change addr
    auto oldBreakPoints = breakpoint_handler->saveBreakpoints(*debug_info->getSymbolTable());
    breakpoint_handler->refreshPid(process->getPid());

    // We must re-read the symbols
    // While the path may not have changed, the user may have recompiled the program
    // in between, so this is a must
    readSymbols();

    // We update the breakPoint table with new addresses
    breakpoint_handler->refreshBreakPoint(*debug_info->getSymbolTable(), oldBreakPoints);

    signal_handler->reset(process.get());
    return true;
  }

  bool ProcessTracer::readSymbols() {
    if (process->getStatus() != Process::Status::kStopped) return false;

    // Read the static symbol table
    ELFFile elf(executable_path);
    const auto* symbolTable = elf.getDebugInfo()->getSymbolTable();
    const Symbol* _start_symbol = symbolTable->operator[]("_start");

    breakpoint_handler->add(*_start_symbol);

    // The process automatically stops at launch when it is traced
    // We resume it
    process->resume();

    waitpid(process->getPid(), nullptr, 0);

    unsigned long rip = ptrace(PTRACE_PEEKUSER, process->getPid(), 8 * RIP, NULL);
    if (not breakpoint_handler->isBreakPoint(rip - 1))
      throw std::runtime_error("Program crashed before _start");


    elf.parseDynamicSymbols(*process);
    debug_info = elf.yieldDebugInfo();

    breakpoint_handler->resetBreakpoint();

    breakpoint_handler->remove(*_start_symbol);
    process->updateStatus(Process::Status::kStopped);
    return debug_info != nullptr;
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

#include "ProcessTracer.h"

#include "RegistersSnapshot.h"
#include <tscl.hpp>
#include <utility>

namespace ldb {

  std::unique_ptr<ProcessTracer> ProcessTracer::fromCommand(const std::string& command,
                                                            const std::vector<std::string>& args) {
    auto process = Process::fromCommand(command, args, true);
    if (not process) { return nullptr; }

    // The process automatically stops at launch when it is traced
    // We resume it
    waitpid(process->getPid(), nullptr, 0);
    process->resume();

    return std::make_unique<ProcessTracer>(std::move(process), command, args);
  }

  ProcessTracer::ProcessTracer(std::unique_ptr<Process>&& process, std::string executable,
                               std::vector<std::string> args)
      : process(std::move(process)), executable_path(std::move(executable)),
        arguments(std::move(args)) {

    if (not this->process) throw std::runtime_error("ProcessTracer: cannot trace a null process");
  }

  bool ProcessTracer::restart() {
    process = Process::fromCommand(executable_path, arguments, true);
    if (not process) throw std::runtime_error("ProcessTracer: failed to reset the process");
    waitpid(process->getPid(), nullptr, 0);
    process->resume();
    if (signal_handler) signal_handler->reset();

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

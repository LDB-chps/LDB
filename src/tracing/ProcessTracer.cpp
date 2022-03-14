#include "ProcessTracer.h"

#include "RegistersSnapshot.h"
#include "sys/wait.h"
#include <unistd.h>
#include <utility>

namespace ldb {

  std::unique_ptr<ProcessTracer> ProcessTracer::fromCommand(const std::string& command,
                                                            const std::vector<std::string>& args) {
    auto process = Process::fromCommand(command, args, true);
    if (not process) { return nullptr; }

    // The process automatically stops at launch when it is traced
    // We resume it
    process->waitNextEvent();
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
    if (not process) throw std::runtime_error("ProcessTracer: failed to restart the process");
    process->waitNextEvent();
    process->resume();

    return true;
  }

  std::unique_ptr<RegistersSnapshot> ProcessTracer::getRegistersSnapshot() {
    std::shared_lock<std::shared_mutex> lock(main_mutex);
    if (not isProbeableStatus(process->getStatus())) { return {}; }

    return std::make_unique<RegistersSnapshot>(*process);
  }

  const std::string& ProcessTracer::getExecutable() {
    // The executable_path is immutable, so we can safely return a copy without acquiring the mutex
    return executable_path;
  }

  std::string ProcessTracer::getCurrentFile() {
    std::shared_lock<std::shared_mutex> lock(main_mutex);
    if (process->getStatus() != Process::Status::kStopped) { return {}; }

    return "";
  }

  long ProcessTracer::getCurrentLineNumber() {
    std::shared_lock<std::shared_mutex> lock(main_mutex);
    if (process->getStatus() != Process::Status::kStopped) { return {}; }

    return -1;
  }

  std::string ProcessTracer::getCurrentFunctionName() {
    std::shared_lock<std::shared_mutex> lock(main_mutex);
    if (process->getStatus() != Process::Status::kStopped) { return {}; }
    return "";
  }

  /**
   * @brief Block until the process receives a signal or terminates
   * @return The status of the process after the wait
   */
  Process::Status ProcessTracer::waitNextEvent() {
    return process->waitNextEvent();
  }

  /*
  std::unique_ptr<StackTrace> ProcessTracer::getStackTrace() {
    std::shared_lock<std::shared_mutex> lock(main_mutex);
    if (process.getStatus() != Process::Status::Stopped) { return {}; }
    return {};
  }*/

}// namespace ldb

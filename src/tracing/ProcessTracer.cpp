#include "ProcessTracer.h"
#include "RegistersSnapshot.h"


namespace ldb {

  std::unique_ptr<ProcessTracer> ProcessTracer::fromCommand(const std::string& command,
                                                            const std::string& args) {
    auto process = Process::fromCommand(command, args);
    if (not process) { return nullptr; }

    return std::make_unique<ProcessTracer>(std::move(*process), command);
  }

  ProcessTracer::ProcessTracer(Process&& process, const std::string& executable)
      : process(std::move(process)), executable_path(executable) {;
  }

  std::unique_ptr<RegistersSnapshot> ProcessTracer::getRegistersSnapshot() {
    std::shared_lock<std::shared_mutex> lock(main_mutex);
    if (process.getStatus() != Process::Status::Stopped) { return {}; }

    return std::make_unique<RegistersSnapshot>(process);
  }

  std::string ProcessTracer::getExecutable() {
    // The executable_path is immovable, so we can safely return a copy without acquiring the mutex
    return executable_path;
  }

  std::string ProcessTracer::getCurrentFile() {
    std::shared_lock<std::shared_mutex> lock(main_mutex);
    if (process.getStatus() != Process::Status::Stopped) { return {}; }

    return "";
  }

  long ProcessTracer::getCurrentLineNumber() {
    std::shared_lock<std::shared_mutex> lock(main_mutex);
    if (process.getStatus() != Process::Status::Stopped) { return {}; }

    return -1;
  }

  std::string ProcessTracer::getCurrentFunctionName() {
    std::shared_lock<std::shared_mutex> lock(main_mutex);
    if (process.getStatus() != Process::Status::Stopped) { return {}; }
    return "";
  }

  /*
  std::unique_ptr<StackTrace> ProcessTracer::getStackTrace() {
    std::shared_lock<std::shared_mutex> lock(main_mutex);
    if (process.getStatus() != Process::Status::Stopped) { return {}; }
    return {};
  }
  */

}// namespace ldb

#include "ProcessTracer.h"

#include <utility>
#include "RegistersSnapshot.h"
#include <unistd.h>
#include "sys/wait.h"

namespace ldb {

  std::unique_ptr<ProcessTracer> ProcessTracer::fromCommand(const std::string& command,
                                                            const std::string& args) {
    auto process = Process::fromCommand(command, args, true);
    if (not process) { return nullptr; }

    process->wait();
    // The process automatically stops at launch when it is traced
    // We resume it
    process->resume();

    return std::make_unique<ProcessTracer>(std::move(*process), command);
  }

  ProcessTracer::ProcessTracer(Process&& process, std::string  executable)
      : process(std::move(process)), executable_path(std::move(executable)) {;
  }

  std::unique_ptr<RegistersSnapshot> ProcessTracer::getRegistersSnapshot() {
    std::shared_lock<std::shared_mutex> lock(main_mutex);
    if (process.getStatus() != Process::Status::kStopped) { return {}; }

    return std::make_unique<RegistersSnapshot>(process);
  }

  std::string ProcessTracer::getExecutable() {
    // The executable_path is immutable, so we can safely return a copy without acquiring the mutex
    return executable_path;
  }

  std::string ProcessTracer::getCurrentFile() {
    std::shared_lock<std::shared_mutex> lock(main_mutex);
    if (process.getStatus() != Process::Status::kStopped) { return {}; }

    return "";
  }

  long ProcessTracer::getCurrentLineNumber() {
    std::shared_lock<std::shared_mutex> lock(main_mutex);
    if (process.getStatus() != Process::Status::kStopped) { return {}; }

    return -1;
  }

  std::string ProcessTracer::getCurrentFunctionName() {
    std::shared_lock<std::shared_mutex> lock(main_mutex);
    if (process.getStatus() != Process::Status::kStopped) { return {}; }
    return "";
  }

  /**
     * @brief Block until the process receives a signal or terminates
     * @return The status of the process after the wait
   */
  Process::Status ProcessTracer::waitNextEvent() {
    return process.waitNextEvent();
  }

  /*
  std::unique_ptr<StackTrace> ProcessTracer::getStackTrace() {
    std::shared_lock<std::shared_mutex> lock(main_mutex);
    if (process.getStatus() != Process::Status::Stopped) { return {}; }
    return {};
  }*/

}// namespace ldb


#include "processTracer.h"


namespace ldb {

  std::unique_ptr<ProcessTracer> ProcessTracer::attachFromCommand(const std::string& command,
                                                                  std::string& args) {
    auto res = std::make_unique<ProcessTracer>();
    res->process = Process::fromCommand(command, args);
    res->executable_path = command;

    return res;
  }

  std::vector<std::string> ProcessTracer::getRegistersValues() {
    std::shared_lock<std::shared_mutex> lock(main_mutex);
    if (process->isRunning()) { return {}; }

    return {};
  }

  std::vector<std::string> ProcessTracer::getGlobalVariablesValues() {
    std::shared_lock<std::shared_mutex> lock(main_mutex);
    if (process->isRunning()) { return {}; }

    return {};
  }

  std::filesystem::path ProcessTracer::getExecutablePath() {
    if (process->isRunning()) { return ""; }

    return "";
  }

  std::filesystem::path ProcessTracer::getCurrentFilePath() {
    std::shared_lock<std::shared_mutex> lock(main_mutex);
    if (process->isRunning()) { return ""; }

    return "";
  }

  long ProcessTracer::getCurrentLineNumber() {
    std::shared_lock<std::shared_mutex> lock(main_mutex);
    if (process->isRunning()) { return -1; }

    return -1;
  }

  std::string ProcessTracer::getCurrentFunctionName() {
    std::shared_lock<std::shared_mutex> lock(main_mutex);
    if (process->isRunning()) { return ""; }

    return "";
  }

  std::vector<std::string> ProcessTracer::getStackTrace() {
    std::shared_lock<std::shared_mutex> lock(main_mutex);
    if (process->isRunning()) { return {}; }

    return {};
  }


}// namespace ldb

#pragma once
#include "DebugInfo.h"
#include "Process.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>

namespace ldb {

  // We won't be wrapping libelf or libdwarf, so we'll keep the code simple without using classes
  std::unique_ptr<const DebugInfo> readDebugInfo(const std::filesystem::path& filePath, Process& S);

}// namespace ldb

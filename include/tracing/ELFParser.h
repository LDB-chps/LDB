#pragma once
#include "DebugInfo.h"
#include "Process.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <filesystem>
#include <libelf.h>
#include <utility>
#include <optional>
#include <sstream>

namespace ldb {

  // We won't be wrapping libelf or libdwarf, so we'll keep the code simple without using classes
  std::unique_ptr<const DebugInfo> readDebugInfo(const std::filesystem::path& filePath, Process& S);

  std::optional<std::pair<std::string, size_t>> addr2Line(const std::filesystem::path& path, const Elf64_Addr addr);

}// namespace ldb

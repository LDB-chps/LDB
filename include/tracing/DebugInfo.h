#pragma once
#include "FrameTable.h"
#include "Section.h"
#include "SymbolsTable.h"
#include <elf.h>
#include <iostream>
#include <memory>
#include <vector>
#include <filesystem>

namespace ldb {
  class DebugInfo {
    friend class DebugInfoFactory;

  public:


    DebugInfo(const DebugInfo& other) = delete;
    DebugInfo& operator=(const DebugInfo& other) = delete;

    const FrameTable& getFrameInfos() const {
      return frame_infos;
    }

    const SymbolsTable& getSymbolsTable() const {
      return symbols_table;
    }

  private:
    DebugInfo() = default;

    std::filesystem::path executable_path;

    FrameTable frame_infos;
    SymbolsTable symbols_table;
  };

}// namespace ldb

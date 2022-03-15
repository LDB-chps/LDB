#pragma once
#include "Process.h"
#include "SymbolTable.h"
#include <elf.h>
#include <filesystem>
#include <iostream>
#include <memory>
#include <vector>

namespace ldb {
  class DebugInfo {
  public:
    DebugInfo() = default;

    DebugInfo(const DebugInfo& other) = delete;
    DebugInfo& operator=(const DebugInfo& other) = delete;

    SymbolTable* getSymbolTable() {
      return symbols_table.get();
    }

    const SymbolTable* getSymbolTable() const {
      return symbols_table.get();
    }

    void appendSharedLibraries(std::filesystem::path shlib) {
      shared_libraries.push_back(shlib);
    }

    const std::vector<std::filesystem::path>& getSharedLibraries() const {
      return shared_libraries;
    }

    std::unique_ptr<SymbolTable> yieldSymbolTable() {
      return std::move(symbols_table);
    }

    void setSymbolTable(std::unique_ptr<SymbolTable>&& table) {
      symbols_table = std::move(table);
    }

  private:
    std::filesystem::path executable_path;
    std::unique_ptr<SymbolTable> symbols_table;
    std::vector<std::filesystem::path> shared_libraries;
  };

}// namespace ldb

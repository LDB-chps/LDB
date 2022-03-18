#pragma once
#include "DebugInfo.h"
#include "Process.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <libelf.h>
#include <memory>
#include <optional>
#include <sstream>
#include <utility>

namespace fs = std::filesystem;

namespace ldb {

  /**
   * @brief Helper class to parse an elf file
   * This class is declared locally since the user should only use the readDebugInfo(...) function
   * This allows the function to be stateless and thus thread-safe.
   */
  class ELFFile {
  public:
    /**
     * @brief Builds a new ELFFile object and parses static symbols from the file
     * This functions does not parse dwarf nor dynamic symbols.
     * @param elf_path
     */
    explicit ELFFile(const fs::path& elf_path);

    ~ELFFile();

    /**
     * @brief This object should not be copyable
     * @param other
     */
    ELFFile(const ELFFile& other) = delete;
    ELFFile& operator=(const ELFFile& other) = delete;

    /**
     * @brief Parses the link map of the given process and append the loaded symbols to this
     * object This function does not check the given process is associated  with this ELFFile
     * object. If this is not the case, this is considered undefined behaviour.
     * @param process
     */
    bool parseDynamicSymbols(Process& process);

    bool hasSymbolTable() const {
      return debug_info and debug_info->getSymbolTable();
    }

    std::unique_ptr<DebugInfo> yieldDebugInfo() {
      return std::move(debug_info);
    }

    DebugInfo* getDebugInfo() {
      return debug_info.get();
    }

  private:
    // Load the entire file into memory
    void loadIntoMemory(const fs::path& elf_path);

    // Parse a string table from memory
    std::string parseStringTable(const Elf64_Shdr& str_header);

    // Fill the sections vector
    bool parseSections();

    static std::string parseLiveString(Process& process, Elf64_Addr str_addr);

    std::vector<std::pair<Elf64_Addr, std::string>> parseLinkMap(Process& process,
                                                                 Elf64_Addr link_map_addr);

    // Fill the symbols table
    void parseSymbols();

    // Locate the link map of the given process
    Elf64_Addr locateLinkMap(const Process& process);

    Elf64_Ehdr* header;
    std::unique_ptr<char[]> data;
    size_t data_size = 0;

    std::filesystem::path elf_path;

    std::unique_ptr<DebugInfo> debug_info;
    std::vector<Elf64_Shdr> sections;

    bool badbit;
  };

  // We won't be wrapping libelf or libdwarf, so we'll keep the code simple without using classes
  std::unique_ptr<const DebugInfo> readDebugInfo(const std::filesystem::path& filePath, Process& S);

  std::optional<std::pair<std::string, size_t>> addr2Line(const std::filesystem::path& path,
                                                          const Elf64_Addr addr);

}// namespace ldb

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
    /**
     * @brief Load the entire file into memory
     * 
     * @param elf_path path 
     */
    void loadIntoMemory(const fs::path& elf_path);

    /**
     * @brief Parse a string table from memory
     * 
     * @param str_header String table header
     * @return std::string the string table content
     */
    std::string parseStringTable(const Elf64_Shdr& str_header);

    /**
     * @brief Fill the sections vector
     * 
     * @return true
     * @return false 
     */
    bool parseSections();

    /**
     * @brief Read string in the process memory
     * 
     * @param process porcess to parse
     * @param str_addr address of the string
     * @return std::string string read
     */
    static std::string parseLiveString(Process& process, Elf64_Addr str_addr);

    /**
     * @brief 
     * 
     * @param process 
     * @param link_map_addr 
     * @return std::vector<std::pair<Elf64_Addr, std::string>> 
     */
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


  /**
   * @brief Convert a address to a line in file
   * 
   * @param path 
   * @param addr 
   * @return std::optional<std::pair<std::string, size_t>> 
   */
  std::optional<std::pair<std::string, size_t>> addr2Line(const std::filesystem::path& path,
                                                          const Elf64_Addr addr);

}// namespace ldb

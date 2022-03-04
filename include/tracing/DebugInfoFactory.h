#pragma once

#include "DebugInfo.h"
#include <filesystem>

namespace ldb {
  /**
   * @brief Constructs DebugInfo for a given program by recursively parsing
   * ELF and DWARF debug information
   * DebugInfo objects built by this objects are immutable.
   */
  class DebugInfoFactory {
  public:
    /**
     * @brief Constructs debug info for a given program
     *
     * @param executable_path Path to the executable
     * @param pid The pid of the program to be used for online debug info, such as dynamic symbol resolution
     * If pid is 0, then only offline debug info will be constructed
     * @return DebugInfo object
     */
    static std::unique_ptr<const DebugInfo> load(std::filesystem::path const& executable_path,
                                                 pid_t pid);

  private:

    /**
     * @brief Recursively parse an file and merges its debug information with the DebugInfo object
     * @param elf_path The path to the ELF file to parse
     * @param debug_info The DebugInfo object to fill
     * @param pid The pid of the program to be used for online debug info, such as dynamic symbol resolution
     * If pid is 0, then only offline debug info will be constructed
     * @param relocation_base Relocate the debug info found in this elf file to this address
     * This is used to relocate debug info from a shared library to the address where it is loaded
     * @return True if the file was parsed successfully, false otherwise
     */
    static bool parseElf(std::filesystem::path const& elf_path, DebugInfo& debug_info, pid_t pid,
                         size_t relocation_base);
  };

}// namespace ldb

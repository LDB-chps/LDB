#pragma once
#include <elf.h>
#include <filesystem>
#include <iostream>
#include <optional>

namespace ldb {
  /**
   * @brief Represents a symbol in the ELF file, such as a function, global variable, etc.
   *
   */
  class Symbol {
  public:
    /**
     * @brief Construct a new symbol
     * @param addr The address of this symbol. This may be relative to the source file, or the
     * current binary.
     * @param name The name of this symbol.
     * @param file An optional file path, if it is known
     */
    Symbol(Elf64_Addr addr, std::string name, std::optional<std::filesystem::path> file)
        : addr(addr), name(name), file(file) {}


    /**
     * @brief Relocate this symbol to a new address
     * @param base The base address for the relocation
     */
    void relocate(Elf64_Addr base) {
      addr += base;
    }

    /**
     * @brief Returns the file where this symbols is defined
     * @return Return an optional file path
     */
    const std::optional<std::filesystem::path>& getFile() const {
      return file;
    }

    /**
     * The address returned may be relative to the source file, or the current binary, if the symbol
     * was relocated
     * @return Returns the address of this symbol
     */
    Elf64_Addr getAddress() const {
      return addr;
    }

    /**
     * @return Returns the name of this symbol
     */
    const std::string& getName() const {
      return name;
    }

    /**
     * @brief compares two symbols for equality using their adresses
     * @param other
     * @return
     */
    bool operator==(const Symbol& other) const {
      // Two symbols are equal if they have the same address and name
      return addr == other.addr && name == other.name;
    }

    bool operator!=(const Symbol& other) const {
      return not(*this == other);
    }

  private:
    std::optional<std::filesystem::path> file;
    Elf64_Addr addr;
    std::string name;
  };

}// namespace ldb

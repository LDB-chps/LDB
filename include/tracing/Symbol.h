#pragma once
#include <elf.h>
#include <filesystem>
#include <iostream>
#include <optional>
#include <vector>

namespace ldb {

  /**
   * @brief Represents a symbol in the ELF file, such as a function, global variable, etc.
   *
   */
  class Symbol {
    friend std::ostream& operator<<(std::ostream& os, const Symbol& symbol);
  public:

    Symbol(const std::string& strName, const std::string& strType)
        : addr(0), name(strName), line(0), type(strType) {}

    /**
     * @brief Construct a new symbol
     * @param addr The address of this symbol. This may be relative to the source file, or the
     * current binary.
     * @param name The name of this symbol.
     * @param file An optional file path, if it is known
     */
    Symbol(Elf64_Addr addr, std::string name, std::filesystem::path file)
        : addr(addr), name(name), file(file) {}

    /**
     * @brief Relocate this symbol to a new address
     * @param base The base address for the relocation
     */
    void relocate(Elf64_Addr base) {
      addr += base;
    }

    const std::filesystem::path& getFile() const {
      return file;
    }

    void setFile(const std::string& strFile) {
      file = std::filesystem::path(strFile);
    }

    Elf64_Addr getAddress() const {
      return addr;
    }

    const std::string& getName() const {
      return name;
    }

    void setName(const std::string& strName) {
      name = strName;
    }

    const size_t getLine() const {
      return line;
    }

    void setLine(const size_t l) {
      line = line;
    }

    const std::string& getType() const {
      return type;
    }

    void setType(const std::string& strType) {
      type = strType;
    }

    const std::vector<Symbol>& getArgs() const {
      return args;
    }

    std::vector<Symbol>& getArgs() {
      return args;
    }

    const std::vector<Symbol>& getVars() const {
      return vars;
    }

    std::vector<Symbol>& getVars() {
      return vars;
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
    std::filesystem::path file;
    Elf64_Addr addr;
    std::string name;
    size_t line;
    std::string type;
    std::vector<Symbol> args;
    std::vector<Symbol> vars;
  };
}// namespace ldb

#pragma once
#include "Symbol.h"
#include <iostream>
#include <vector>


namespace ldb {

  /**
   * @brief A table of symbols that can be used to resolve symbols.
   *
   * This table is compounded of multiple SymbolList objects, than can be loaded from multiple files
   */
  class SymbolTable {
  public:
    friend std::ostream& operator<<(std::ostream& os, const SymbolTable& table);

    SymbolTable() = default;
    explicit SymbolTable(size_t size) {
      symbols.reserve(size);
    }

    bool isEmpty() const {
      return symbols.empty();
    }

    void shrinkToFit();

    void relocate(Elf64_Addr);

    Symbol& push_back(const Symbol& symbol) {
      symbols.push_back(symbol);
      return symbols.back();
    }

    template<typename... Args>
    Symbol& emplace_back(Args&&... args) {
      symbols.emplace_back(args...);
      return symbols.back();
    }

    Symbol* operator[](const std::string& name);
    const Symbol* operator[](const std::string& name) const;

    Symbol* operator[](Elf64_Addr name);
    const Symbol* operator[](Elf64_Addr name) const;

    Symbol* findClosestFunction(Elf64_Addr addr);
    const Symbol* findClosestFunction(Elf64_Addr addr) const;

    std::string getObjectFileOf(const Elf64_Addr addr) const;

    void join(std::unique_ptr<SymbolTable>&& other);

    using iterator = std::vector<Symbol>::iterator;
    using const_iterator = std::vector<Symbol>::const_iterator;

    iterator begin() {
      return symbols.begin();
    }
    iterator end() {
      return symbols.end();
    }

    const_iterator begin() const {
      return symbols.begin();
    }
    const_iterator end() const {
      return symbols.end();
    }

  private:
    std::vector<Symbol> symbols;
    std::unique_ptr<SymbolTable> next;
    std::string file;
    std::filesystem::path object_file;
  };


}// namespace ldb

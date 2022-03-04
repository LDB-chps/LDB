#pragma once

#include "Symbol.h"
#include <vector>
#include <unordered_set>


namespace ldb {
  class SymbolList {
  public:

    bool push_back(const Symbol& symbol);
    void relocate(Elf64_Addr base);

    using iterator = std::vector<Symbol>::iterator;
    using const_iterator = std::vector<Symbol>::const_iterator;

    Symbol* operator[](const std::string& name);
    const Symbol* operator[](const std::string& name) const;

    Symbol* operator[](Elf64_Addr addr);
    const Symbol* operator[](Elf64_Addr addr) const;

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

    const_iterator cbegin() const {
      return symbols.cbegin();
    }
    const_iterator cend() const {
      return symbols.cend();
    }

  private:
    std::vector<Symbol> symbols;
  };
}// namespace ldb

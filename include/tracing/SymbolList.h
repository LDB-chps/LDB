#pragma once

#include "Symbol.h"
#include <vector>
#include <unordered_set>


namespace ldb {
  class SymbolList {
  public:
    using iterator = std::unordered_set<Symbol>::iterator;
    using const_iterator = std::unordered_set<Symbol>::const_iterator;

    bool push_back(const Symbol& symbol);
    void relocate(Elf64_Addr base);

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
    std::unordered_set<Symbol> symbols;
  };
}// namespace ldb

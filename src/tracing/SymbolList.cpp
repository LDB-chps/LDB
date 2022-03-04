#include "SymbolList.h"
#include <algorithm>
#include <execution>


namespace ldb {
  bool SymbolList::push_back(const Symbol& symbol) {
    // Symbols should be unique
    auto it = std::find_if(std::execution::par_unseq, symbols.begin(), symbols.end(),
                           [&](const Symbol& sym) { return symbol == sym; });
    if (it == symbols.end()) {
      symbols.push_back(symbol);
      return true;
    }
    return false;
  }

  void SymbolList::relocate(Elf64_Addr base) {
    std::for_each(std::execution::par_unseq, symbols.begin(), symbols.end(),
                  [&](Symbol& symbol) { symbol.relocate(base); });
  }

  Symbol* SymbolList::operator[](const std::string& name) {
    auto it = std::find_if(std::execution::par_unseq, symbols.begin(), symbols.end(),
                           [&](const Symbol& symbol) { return symbol.getName() == name; });
    if (it == symbols.end()) { return nullptr; }
    return &*it;
  }

  const Symbol* SymbolList::operator[](const std::string& name) const {
    auto it = std::find_if(std::execution::par_unseq, symbols.begin(), symbols.end(),
                           [&](const Symbol& symbol) { return symbol.getName() == name; });
    if (it == symbols.end()) { return nullptr; }
    return &*it;
  }

  Symbol* SymbolList::operator[](Elf64_Addr addr) {
    auto it = std::find_if(std::execution::par_unseq, symbols.begin(), symbols.end(),
                           [&](const Symbol& symbol) { return symbol.getAddress() == addr; });
    if (it == symbols.end()) { return nullptr; }
    return &*it;
  }

  const Symbol* SymbolList::operator[](Elf64_Addr addr) const {
    auto it = std::find_if(std::execution::par_unseq, symbols.begin(), symbols.end(),
                           [&](const Symbol& symbol) { return symbol.getAddress() == addr; });
    if (it == symbols.end()) { return nullptr; }
    return &*it;
  }

}// namespace ldb
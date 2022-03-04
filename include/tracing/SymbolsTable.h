#pragma once
#include "Symbol.h"
#include "SymbolList.h"
#include <iostream>
#include <vector>


namespace ldb {

  /**
   * @brief A table of symbols that can be used to resolve symbols.
   */
  class SymbolsTable {
  public:

    Symbol* find(const std::string& name);
    const Symbol* find(const std::string& name) const;

    Symbol* find(Elf64_Addr addr);
    const Symbol* find(Elf64_Addr addr) const;

    Symbol* findClosestFunction(Elf64_Addr addr);
    const Symbol* findClosestFunction(Elf64_Addr addr) const;

    void join(const std::shared_ptr<SymbolList>& other) {
      symbols_list.push_back(other);
    }

  private:
    std::vector<std::shared_ptr<SymbolList>> symbols_list;
  };

}// namespace ldb

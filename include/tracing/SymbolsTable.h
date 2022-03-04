#pragma once
#include "Symbol.h"
#include "SymbolList.h"
#include <iostream>
#include <vector>


namespace ldb {

  /**
   * @brief A table of symbols that can be used to resolve symbols.
   *
   * This table is compounded of multiple SymbolList objects, than can be loaded from multiple files
   */
  class SymbolsTable {
  public:

    friend std::ostream& operator<<(std::ostream& os, const SymbolsTable& table);

    /**
     * @brief Lookup the symbol of given name in the table
     * @param name The name of the symbol to lookup
     * @return A pointer to the symbol if found, nullptr otherwise
     */
    Symbol* find(const std::string& name);

    /**
     * @brief Lookup the symbol of given name in the table
     * @param name The name of the symbol to lookup
     * @return A pointer to the symbol if found, nullptr otherwise
     */
    const Symbol* find(const std::string& name) const;

    /**
     * @brief Lookup the symbol of given address in the table
     * @param name The name of the symbol to lookup
     * @return A pointer to the symbol if found, nullptr otherwise
     */
    Symbol* find(Elf64_Addr addr);

    /**
     * @brief Lookup the symbol of given address in the table
     * @param name The name of the symbol to lookup
     * @return A pointer to the symbol if found, nullptr otherwise
     */
    const Symbol* find(Elf64_Addr addr) const;

    /**
     * @brief Lookups the closest function symbol to the given address
     * This can be used to locate where the program currently is
     *
     * If the function containing this address is not in the table, the return function can be
     * totally wrong, and this cannot be detected.
     *
     * @param addr The address to lookup
     * @return A pointer to the function symbol if found, nullptr otherwise
     */
    Symbol* findClosestFunction(Elf64_Addr addr);
    const Symbol* findClosestFunction(Elf64_Addr addr) const;

    void join(const std::shared_ptr<SymbolList>& other) {
      symbols_list.push_back(other);
    }

  private:
    std::vector<std::shared_ptr<SymbolList>> symbols_list;
  };



}// namespace ldb

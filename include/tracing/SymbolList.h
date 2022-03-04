#pragma once

#include "Symbol.h"
#include <unordered_set>
#include <vector>


namespace ldb {
  /**
   * @brief This class represents a list of symbols loaded from an elf file
   *
   * Multiple symbols list combines together to form a SymbolTable.
   */
  class SymbolList {
  public:
    /**
     * @brief Append a symbol to the end of this symbol list
     * @param symbol The symbol to append
     * @return True if the symbol was appended, false if the symbol was already in the list
     */
    bool push_back(const Symbol& symbol);

    template<typename... Args>
    bool emplace_back(Args... args) {
      return push_back(Symbol(args...));
    }

    /**
     * @brief Relocates every symbol in this list to the given address
     * @param base The base address to relocate to
     */
    void relocate(Elf64_Addr base);

    using iterator = std::vector<Symbol>::iterator;
    using const_iterator = std::vector<Symbol>::const_iterator;

    /**
     * @brief Lookup the symbol with the given name in the list
     * @return The symbol if found, or nullptr if not found
     */
    Symbol* operator[](const std::string& name);

    /**
     * @brief Lookup the symbol with the given name in the list
     * @return The symbol if found, or nullptr if not found
     */
    const Symbol* operator[](const std::string& name) const;

    /**
     * @brief Lookup the symbol with the given address in the list
     * @return  The symbol if found, or nullptr if not found
     */
    Symbol* operator[](Elf64_Addr addr);

    /**
     * @brief Lookup the symbol with the given address in the list
     * @return  The symbol if found, or nullptr if not found
     */
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

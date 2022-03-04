#include "SymbolsTable.h"
#include <atomic>
#include <thread>
namespace ldb {

  Symbol* SymbolsTable::find(const std::string& name) {

    for (auto& list_p : symbols_list) {
      auto& list = *list_p;
      auto res = list[name];
      if (res != nullptr) { return res; }
    }
    return nullptr;
  }

  const Symbol* SymbolsTable::find(const std::string& name) const {
    for (auto& list_p : symbols_list) {
      auto& list = *list_p;
      auto res = list[name];
      if (res != nullptr) { return res; }
    }
    return nullptr;
  }

  Symbol* SymbolsTable::find(Elf64_Addr addr) {
    for (auto& list_p : symbols_list) {
      auto& list = *list_p;
      auto res = list[addr];
      if (res != nullptr) { return res; }
    }
    return nullptr;
  }

  const Symbol* SymbolsTable::find(Elf64_Addr addr) const {

    for (auto& list_p : symbols_list) {
      auto& list = *list_p;
      auto res = list[addr];
      if (res != nullptr) { return res; }
    }
    return nullptr;
  }

  Symbol* SymbolsTable::findClosestFunction(Elf64_Addr addr) {
    Symbol* closest = nullptr;

    for (auto& list : symbols_list) {
      for (auto& sym : *list) {
        if (sym.getAddress() < addr and
            (closest == nullptr || sym.getAddress() < closest->getAddress()))
          closest = &sym;
      }
    }

    // This could indicate an unresolved symbol
    if (closest->getAddress() == 0) { return nullptr; }

    return closest;
  }

  const Symbol* SymbolsTable::findClosestFunction(Elf64_Addr addr) const {
    const Symbol* closest = nullptr;

    for (auto& list : symbols_list) {
      for (auto& sym : *list) {
        if (sym.getAddress() < addr and
            (closest == nullptr || sym.getAddress() < closest->getAddress()))
          closest = &sym;
      }
    }
    return closest;
  }


  std::ostream& operator<<(std::ostream& os, const SymbolsTable& table) {
    for (auto& list : table.symbols_list) {
      for (auto& sym : *list) { os << sym << std::endl; }
    }
    return os;
  }

}// namespace ldb
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

  const Symbol* SymbolsTable::findClosestFunction(Elf64_Addr addr) const {
    const Symbol* closest = nullptr;

    for (auto& list : symbols_list) {
      for (auto& sym : *list) {
        if (sym.getType() == SymbolType::kFunction and sym.getAddress() < addr and
            (closest == nullptr || sym.getAddress() < closest->getAddress()))
          closest = &sym;
      }
    }
    return closest;
  }

}// namespace ldb
#include "SymbolTable.h"
#include <atomic>
#include <thread>
namespace ldb {

  Symbol* SymbolTable::operator[](const std::string& name) {

    for (auto& it : symbols) {
      if (it.getName() == name) { return &it; }
    }

    if (next) { return next->operator[](name); }

    return nullptr;
  }

  const Symbol* SymbolTable::operator[](const std::string& name) const {
    for (auto& it : symbols) {
      if (it.getName() == name) { return &it; }
    }

    if (next) { return next->operator[](name); }

    return nullptr;
  }

  Symbol* SymbolTable::operator[](Elf64_Addr addr) {
    for (auto& it : symbols) {
      if (it.getAddress() == addr) { return &it; }
    }

    if (next) { return next->operator[](addr); }

    return nullptr;
  }

  const Symbol* SymbolTable::operator[](Elf64_Addr addr) const {
    for (auto& it : symbols) {
      if (it.getAddress() == addr) { return &it; }
    }

    if (next) { return next->operator[](addr); }

    return nullptr;
  }

  Symbol* SymbolTable::findClosestFunction(Elf64_Addr addr) {
    Symbol* closest = nullptr;

    SymbolTable* table = nullptr;

    for (table = this; table; table = table->next.get()) {
      for (auto& sym : table->symbols) {
        if (sym.getAddress() < addr and
            (closest == nullptr || sym.getAddress() < closest->getAddress()))
          closest = &sym;
      }
    }

    // Unresolved symbols
    if (not closest or closest->getAddress() == 0) { return nullptr; }

    return closest;
  }

  const Symbol* SymbolTable::findClosestFunction(Elf64_Addr addr) const {
    const Symbol* closest = nullptr;

    const SymbolTable* table = nullptr;

    for (table = this; table; table = table->next.get()) {
      for (auto& sym : table->symbols) {
        if (sym.getAddress() < addr and
            (closest == nullptr || sym.getAddress() < closest->getAddress()))
          closest = &sym;
      }
    }

    // Unresolved symbols
    if (not closest or closest->getAddress() == 0) { return nullptr; }

    return closest;
  }


  std::ostream& operator<<(std::ostream& os, const SymbolTable& table) {

    const SymbolTable* ptr = nullptr;

    for (ptr = &table; ptr != nullptr; ptr = ptr->next.get()) {
      for (auto& sym : ptr->symbols) { os << sym << std::endl; }
    }
    return os;
  }

}// namespace ldb
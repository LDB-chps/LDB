#include "SymbolTable.h"
#include <atomic>
#include <thread>
namespace ldb {

  void SymbolTable::shrinkToFit() {
    for (SymbolTable* curr = this; curr != nullptr; curr = curr->next.get()) {
      curr->symbols.shrink_to_fit();
    }
  }

  void SymbolTable::relocate(Elf64_Addr addr) {
    for (SymbolTable* curr = this; curr != nullptr; curr = curr->next.get()) {
      for (auto& sym : curr->symbols) sym.relocate(addr);
    }
  }

  Symbol* SymbolTable::operator[](const std::string& name) {
    for (SymbolTable* curr = this; curr != nullptr; curr = curr->next.get()) {
      for (auto& sym : curr->symbols) {
        if (sym.getName() == name) { return &sym; }
      }
    }
    return nullptr;
  }

  const Symbol* SymbolTable::operator[](const std::string& name) const {
    for (const SymbolTable* curr = this; curr != nullptr; curr = curr->next.get()) {
      for (const auto& sym : curr->symbols) {
        if (sym.getName() == name) { return &sym; }
      }
    }
    return nullptr;
  }

  Symbol* SymbolTable::operator[](Elf64_Addr addr) {
    for (SymbolTable* curr = this; curr != nullptr; curr = curr->next.get()) {
      for (auto& sym : curr->symbols) {
        if (sym.getAddress() == addr) { return &sym; }
      }
    }
    return nullptr;
  }

  const Symbol* SymbolTable::operator[](Elf64_Addr addr) const {
    for (const SymbolTable* curr = this; curr != nullptr; curr = curr->next.get()) {
      for (const auto& sym : curr->symbols) {
        if (sym.getAddress() == addr) { return &sym; }
      }
    }
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

  std::string SymbolTable::getObjectFileOf(const Elf64_Addr addr) const {
    for (const SymbolTable* curr = this; curr != nullptr; curr = curr->next.get()) {
      for (const auto& sym : curr->symbols) {
        if (sym.getAddress() == addr) { return curr->object_file; }
      }
    }
    return nullptr;
  }


  void SymbolTable::join(std::unique_ptr<SymbolTable>&& other) {
    if (not other) { return; }

    // Append the new table at the end of the list
    SymbolTable* curr = nullptr;
    for (curr = this; this->next; curr = curr->next.get())
      ;
    curr->next = std::move(other);
  }


  std::ostream& operator<<(std::ostream& os, const SymbolTable& table) {

    const SymbolTable* ptr = nullptr;

    for (ptr = &table; ptr != nullptr; ptr = ptr->next.get()) {
      for (auto& sym : ptr->symbols) { os << sym << std::endl; }
    }
    return os;
  }

}// namespace ldb
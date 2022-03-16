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
    base_address = addr;
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

  std::pair<const Symbol*, const SymbolTable*> SymbolTable::findInTable(Elf64_Addr addr) const {
    for (const SymbolTable* curr = this; curr != nullptr; curr = curr->next.get()) {
      for (const auto& sym : curr->symbols) {
        if (sym.getAddress() == addr) { return {&sym, curr}; }
      }
    }
    return {nullptr, nullptr};
  }

  void SymbolTable::join(std::unique_ptr<SymbolTable>&& other) {
    if (not other) { return; }

    // Append the new table at the end of the list
    SymbolTable* curr = nullptr;
    for (curr = this; curr->next; curr = curr->next.get())
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
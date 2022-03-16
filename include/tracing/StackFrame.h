#pragma once
#include "Symbol.h"
#include <iostream>

namespace ldb {

  class StackFrame {
  public:
    StackFrame(Elf64_Addr addr, Elf64_Off offset, const Symbol* symbol);
    StackFrame(std::string fun_name, Elf64_Addr addr, Elf64_Off offset);

    const std::string& getFunctionName() const {
      if (symbol) {
        return symbol->getName();
      } else {
        return name;
      }
    }

    const Symbol* getSymbol() const {
      return symbol;
    }

    Elf64_Addr getAddress() const {
      return address;
    }

    Elf64_Off getOffset() const {
      return offset;
    }

  private:
    std::string name;
    Elf64_Addr address;
    Elf64_Off offset;
    const Symbol* symbol;
  };

}// namespace ldb

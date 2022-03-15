#pragma once
#include "Symbol.h"
#include <iostream>

namespace ldb {

  class StackFrame {
  public:
    StackFrame(std::string function_name, Elf64_Addr addr, Elf64_Off offset, Symbol* symbol);

    const std::string& getFunctionName() const {
      return function_name;
    }

    Elf64_Addr getAddress() const {
      return address;
    }

    Elf64_Off getOffset() const {
      return offset;
    }

    Symbol* getSymbol() const {
      return symbol;
    }

  private:
    std::string function_name;
    Elf64_Addr address;
    Elf64_Off offset;
    Symbol* symbol;
  };

}// namespace ldb

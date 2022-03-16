#include "StackFrame.h"

#include <utility>


namespace ldb {
  StackFrame::StackFrame(Elf64_Addr addr, Elf64_Off offset, const Symbol* symbol)
      : address(addr), offset(offset), symbol(symbol) {}

  StackFrame::StackFrame(std::string fun_name, Elf64_Addr addr, Elf64_Off offset)
      : name(std::move(fun_name)), address(addr), offset(offset), symbol(nullptr) {}

}// namespace ldb

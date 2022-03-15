#include "StackFrame.h"

#include <utility>


namespace ldb {
  StackFrame::StackFrame(std::string function_name, Elf64_Addr addr, Elf64_Off offset,
                         Symbol* symbol)
      : function_name(std::move(function_name)), address(addr), offset(offset), symbol(symbol) {}

}// namespace ldb

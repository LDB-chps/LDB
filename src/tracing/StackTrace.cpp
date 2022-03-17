#include "StackTrace.h"
#include "ProcessTracer.h"
#include <libunwind-ptrace.h>
#include <libunwind.h>

namespace ldb {
  StackTrace::StackTrace(ProcessTracer& tracer) {
    pid_t pid = tracer.getProcess().getPid();
    unw_addr_space_t as = unw_create_addr_space(&_UPT_accessors, 0);

    void* context = _UPT_create(pid);
    unw_cursor_t cursor;

    if (unw_init_remote(&cursor, as, context)) return;

    auto* SymbolTable = tracer.getDebugInfo() ? tracer.getDebugInfo()->getSymbolTable() : nullptr;
    if (not SymbolTable) return;

    bool done = false;
    while (not done) {
      unw_word_t offset, pc;
      char sym[2048];
      if (unw_get_reg(&cursor, UNW_REG_IP, &pc) != 0) break;

      int res = unw_get_proc_name(&cursor, sym, sizeof(sym), &offset);

      // Search the address in the symbol table
      const Symbol* symbol = (*SymbolTable)[pc - offset];// We must add the offset to the address to
                                                         // get the address of the function
      if (symbol) {
        frames.emplace_back(pc - offset, offset, symbol);
      } else {
        // If we failed to find a symbol, we can resort to using the name provided by libunwind
        if (res == 0) frames.emplace_back(sym, pc - offset, offset);
        else
          frames.emplace_back("????", pc - offset, offset);
      }
      done = unw_step(&cursor) <= 0;
      if (frames.size() > 50) done = true;
    }
  }
}// namespace ldb

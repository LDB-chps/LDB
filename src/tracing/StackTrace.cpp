#include "StackTrace.h"
#include "ProcessTracer.h"
#include <libunwind-ptrace.h>
#include <libunwind.h>

namespace ldb {
  StackTrace::StackTrace(ProcessTracer& tracer) {
    pid_t pid = tracer.getPid();
    unw_addr_space_t as = unw_create_addr_space(&_UPT_accessors, 0);

    void* context = _UPT_create(pid);
    unw_cursor_t cursor;

    if (unw_init_remote(&cursor, as, context)) return;

    bool done = false;
    while (not done) {
      unw_word_t offset, pc;
      char sym[2048];
      if (unw_get_reg(&cursor, UNW_REG_IP, &pc) != 0) break;
      if (unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0) {
        frames.emplace_back(sym, pc, offset, nullptr);
      } else {
        frames.emplace_back("?????", pc, offset, nullptr);
      }
      done = unw_step(&cursor) <= 0;
    }
  }
}// namespace ldb

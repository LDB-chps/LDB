#pragma once
#include "StackFrame.h"
#include "Symbol.h"
#include <vector>

namespace ldb {

  class ProcessTracer;

  /**
   * @brief A stack trace, built from a set of stack frames.
   */
  class StackTrace {
  public:
    StackTrace(ProcessTracer& tracer);

    size_t size() const {
      return frames.size();
    }

    size_t isEmpty() const {
      return frames.empty();
    }

    /**
     * @brief If the stack trace reaches more than fifty frames, it is truncated.
     * @return True if the stack trace is truncated, false otherwise.
     */
    bool isTruncated() const {
      return is_truncated;
    }

    using iterator = std::vector<StackFrame>::iterator;
    using const_iterator = std::vector<StackFrame>::const_iterator;

    iterator begin() {
      return frames.begin();
    }
    iterator end() {
      return frames.end();
    }

    const_iterator begin() const {
      return frames.begin();
    }
    const_iterator end() const {
      return frames.end();
    }

  private:
    std::vector<StackFrame> frames;
    bool is_truncated = false;
  };

}// namespace ldb

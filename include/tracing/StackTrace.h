#pragma once
#include <vector>
#include "Symbol.h"
#include "StackFrame.h"

namespace ldb {

  class ProcessTracer;


  class StackTrace {
  public:
    StackTrace(ProcessTracer& tracer);

    size_t size() const {
      return frames.size();
    }

    size_t isEmpty() const {
      return frames.empty();
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
  };

}// namespace ldb

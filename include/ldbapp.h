#pragma once

#include <memory>

namespace ldb {

  class LDBApp {
  public:
    LDBApp() = default;

    static void run(int argc, char** argv);

  private:
  };
}// namespace ldb

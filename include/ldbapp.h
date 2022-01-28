//
// Created by thukisdo on 28/01/2022.
//

#pragma once

#include <memory>

namespace ldb {
  /*
   * @brief The LDB class is the main class of the library.
   * it is used to handle the subprocess tracing and display loop
   */
  class LDBApp {
  public:
    LDBApp() = default;

    static void run(int argc, char** argv);

  private:
  };
}


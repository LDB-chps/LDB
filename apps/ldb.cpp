#include "ldbapp.h"
#include <iostream>

int main(int argc, char** argv) {
  std::cout << "Hello, World!" << std::endl;
  ldb::LDBApp app;
  app.run(argc, argv);
  return 0;
}
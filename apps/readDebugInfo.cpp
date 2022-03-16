#include <ELFParser.h>
#include <iostream>

#include "DwarfReader.h"

int main(int argc, char* argv[]) {
  // if (argc != 3) {
  //   std::cerr << "Usage: " << argv[0] << " <executable> <pid>" << std::endl;
  //   return 1;
  // }

  // ldb::Process p(std::atoi(argv[2]));
  // auto infos = ldb::readDebugInfo(argv[1], p);
  // if (not infos) return 1;
  // std::cout << *infos->getSymbolTable() << std::endl;

  auto res = ldb::addr2Line(argv[1], std::atoi(argv[2]));

  if (res) {
    std::cout << "file: " << res->first << "\nline: " << res->second << std::endl;
  } else {
    std::cerr << "addr or file don't work" << std::endl;
  }

  return 0;
}
#include <ELFParser.h>
#include <iostream>

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <executable> <pid>" << std::endl;
    return 1;
  }

  ldb::Process p(std::atoi(argv[2]));
  auto infos = ldb::readDebugInfo(argv[1], p);
  if (not infos)
    return 1;
  std::cout << *infos->getSymbolTable() << std::endl;
  return 0;
}
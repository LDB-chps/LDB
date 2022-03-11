#include <ELFReader.h>
#include <iostream>

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <executable> <pid>" << std::endl;
    return 1;
  }

  auto infos = ldb::ELFReader::read(argv[1], std::atoi(argv[2]));
  std::cout << infos->getSymbolsTable() << std::endl;
  return 0;
}
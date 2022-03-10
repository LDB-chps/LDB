#include <ELFReader.h>
#include <iostream>

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <executable>" << std::endl;
    return 1;
  }

  auto infos = ldb::ELFReader::read(argv[1], 37568);
  std::cout << infos->getSymbolsTable() << std::endl;
  return 0;
}
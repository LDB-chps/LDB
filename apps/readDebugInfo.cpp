#include <ELFParser.h>
#include <iostream>

#include "DwarfReader.h"

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <executable> <pid>" << std::endl;
    return 1;
  }

  ldb::Process p(std::atoi(argv[2]));
  auto infos = ldb::readDebugInfo(argv[1], p);
  if (not infos) return 1;
  std::cout << *infos->getSymbolTable() << std::endl;


  int fd = open("/home/johnkyky/Documents/elf/pid", O_RDONLY);
  if (fd == -1) {
    std::cout << "error" << std::endl;
    exit(100);
  }

  ldb::DwarfReader reader(fd);
  reader.populateDwarf(infos.get()->getSymbolTable());

  return 0;
}
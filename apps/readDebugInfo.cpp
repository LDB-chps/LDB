#include <DebugInfoFactory.h>


int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <executable>" << std::endl;
    return 1;
  }

  auto infos = ldb::DebugInfoFactory::load(argv[1], 0);
  std::cout << infos->getSymbolsTable() << std::endl;
  return 0;
}
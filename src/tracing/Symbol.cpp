#include "Symbol.h"

namespace ldb {
  std::ostream& operator<<(std::ostream& os, const Symbol& symbol) {
    os << "addr: " << symbol.addr << " name: " << symbol.name;
    if (symbol.getFile()) { os << " in file: " << symbol.getFile()->string() << std::endl; }
    return os;
  }
}// namespace ldb

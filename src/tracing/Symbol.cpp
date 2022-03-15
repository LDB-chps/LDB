#include "Symbol.h"

namespace ldb {
  std::ostream& operator<<(std::ostream& os, const Symbol& symbol) {
    os << "addr: " << symbol.addr << " name: " << symbol.name << " ";

    for(size_t i = 0; i < symbol.args.size(); i++)
      os << "arg" << i << "(" << symbol.args[i].type << " " << symbol.args[i].name << ") ";
    
    for(size_t i = 0; i < symbol.vars.size(); i++)
      os << "var" << i << "(" << symbol.vars[i].type << " " << symbol.vars[i].name << ") ";

    os << "in file: " << symbol.file << std::endl;
    return os;
  }
}// namespace ldb

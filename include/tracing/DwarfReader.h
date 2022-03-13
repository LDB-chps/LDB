#pragma once

#include "DebugInfo.h"
#include <iostream>
#include <libdwarf/dwarf.h>
#include <libdwarf/libdwarf.h>
#include <map>
#include <sstream>
#include <string>
#include <vector>

// delete it
#include <fcntl.h>


namespace ldb {

  enum class LANGAGE { C, CPP, UNKNOWN };

  struct symbol {
    symbol() = default;
    symbol(const std::string str) : name(str) {}
    std::string name;
  };

  struct param_variable : public symbol {
    param_variable() = default;
    param_variable(const std::string str, const std::string type) : symbol(str), type(type) {}
    std::string type;
  };

  struct variable : public param_variable {
    variable() = default;
    variable(const std::string str, const std::string type) : param_variable(str, type), line(0) {}
    size_t line;
  };

  struct function : public symbol {
    function() = default;
    function(const std::string str) : symbol(str), line(0) {}
    size_t line;
    std::string file;
    param_variable retour;
    std::vector<param_variable> arg;
    std::vector<variable> var;
  };

  struct SYMBOLS {
    SYMBOLS() = default;

    std::vector<function> funs;
    std::vector<variable> vars;
  };


  class DwarfReader {
  public:
    DwarfReader(const int fd);
    ~DwarfReader() = default;

    void populateDwarf(const SymbolTable& symTab);
    const LANGAGE getLangage() { return langsrc; }

  private:
    void read_cu(const SymbolTable& symTab);
    void get_die_and_siblings(Dwarf_Die die);

    void load_langage(Dwarf_Die die);
    void load_file_tabl(Dwarf_Die die);
    void load_basic_type_map(Dwarf_Die die);
    void load_complexe_type_map(Dwarf_Die die);
    void parse_function(Dwarf_Die die, function& info);

    void print_die_data(Dwarf_Die die);

  private:
    Dwarf_Debug dbg;
    LANGAGE langsrc;
    std::vector<std::string> file_tabl;
    std::map<Dwarf_Off, std::string> type_tabl;

    SYMBOLS SYMBOLS_TABL;
  };

}// namespace ldb
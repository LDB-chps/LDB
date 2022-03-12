#pragma once

#include "DebugInfo.h"
#include <iostream>
#include <libdwarf/dwarf.h>
#include <libdwarf/libdwarf.h>
#include <sstream>
#include <string>
#include <vector>
#include <map>

// delete it
#include <fcntl.h>


namespace ldb {

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
    std::vector<param_variable> arg;
    std::vector<variable> var;
  };

  struct SYMBOLS
  {
    SYMBOLS() = default;
    std::vector<function> funs;
    std::vector<variable> vars;
  };


  void populateDwarf(int fd, SymbolTable& symTab);

  static void read_cu(Dwarf_Debug dbg);

  static std::vector<std::string> read_file_tabl(Dwarf_Die cu_die);
  static void load_type_map(Dwarf_Debug dbg, Dwarf_Die in_die);
  static void get_die_and_siblings(Dwarf_Debug dbg, Dwarf_Die in_die, int level);
  static void print_die_data(Dwarf_Debug dbg, Dwarf_Die die, int level);

  static void parse_function(Dwarf_Debug dbg, Dwarf_Die die, function& info);

  static SYMBOLS SYMBOLS_TABL;
  static std::map<Dwarf_Off, std::string> TYPE_TABL;

}// namespace ldb
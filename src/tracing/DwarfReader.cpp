#include "DwarfReader.h"


namespace ldb {

  DwarfReader::DwarfReader(const int fd) : langsrc(LANGAGE::UNKNOWN) {
    int fdd = open("/home/johnkyky/Documents/elf/pid", O_RDONLY);
    if (fdd == -1) {
      std::cout << "error" << std::endl;
      exit(100);
    }

    const int res = dwarf_init(fdd, DW_DLC_READ, nullptr, nullptr, &dbg, nullptr);
    if (res == DW_DLV_ERROR) { throw std::runtime_error("dwarf_init failed"); }
  }

  void DwarfReader::populateDwarf(const SymbolTable& symTab) {
    if (!dbg) { throw std::runtime_error("dwarf dbg not initialized"); }

    read_cu(symTab);

    std::cout << "\n\n\n" << std::endl;
    switch (langsrc) {
      case LANGAGE::C:
        std::cout << "C programme\n" << std::endl;
        break;
      case LANGAGE::CPP:
        std::cout << "CPP programme\n" << std::endl;
        break;
      case LANGAGE::UNKNOWN:
        std::cout << "UNKNOWN programme\n" << std::endl;
        break;
      default:
        break;
    }

    for (auto& file : file_tabl) { std::cout << file << std::endl; }

    std::cout << "\n\n" << std::endl;
    for (auto& i : type_tabl)
      std::cout << std::hex << "<" << i.first << "> " << i.second << std::endl;

    std::cout << "\n\n" << std::endl;
    for (int i = 0; i < SYMBOLS_TABL.funs.size(); i++) {
      std::cout << std::dec << "L." << SYMBOLS_TABL.funs[i].line << " "
                << SYMBOLS_TABL.funs[i].retour.type << " " << SYMBOLS_TABL.funs[i].name << " ";
      for (int j = 0; j < SYMBOLS_TABL.funs[i].arg.size(); j++)
        std::cout << "arg" << j << "(" << SYMBOLS_TABL.funs[i].arg[j].type << " "
                  << SYMBOLS_TABL.funs[i].arg[j].name << ") ";
      for (int j = 0; j < SYMBOLS_TABL.funs[i].var.size(); j++)
        std::cout << "var" << j << "(" << SYMBOLS_TABL.funs[i].var[j].type << " "
                  << SYMBOLS_TABL.funs[i].var[j].name << ") ";
      std::cout << "file(" << SYMBOLS_TABL.funs[i].file << ")" << std::endl;
    }
    for (int i = 0; i < SYMBOLS_TABL.vars.size(); i++) {
      std::cout << "L." << SYMBOLS_TABL.vars[i].line << " var : (" << SYMBOLS_TABL.vars[i].type
                << ") " << SYMBOLS_TABL.vars[i].name << std::endl;
    }
  }

  void DwarfReader::read_cu(const SymbolTable& symTab) {
    Dwarf_Unsigned cu_header_length = 0;
    Dwarf_Half version_stamp = 0;
    Dwarf_Unsigned abbrev_offset = 0;
    Dwarf_Half address_size = 0;
    Dwarf_Unsigned next_cu_header = 0;
    bool done = true;

    while (done) {
      int res = dwarf_next_cu_header(dbg, &cu_header_length, &version_stamp, &abbrev_offset,
                                     &address_size, &next_cu_header, nullptr);

      if (res == DW_DLV_NO_ENTRY) {
        return;
      } else if (res == DW_DLV_ERROR) {
        throw std::runtime_error("read_cu failed");
      }

      Dwarf_Die cur_die = nullptr;
      res = dwarf_siblingof(dbg, nullptr, &cur_die, nullptr);
      if (res == DW_DLV_ERROR) { throw std::runtime_error("dwarf_siblingof failed"); }
      if (res == DW_DLV_NO_ENTRY) { throw std::runtime_error("No DIEs in CU"); }

      load_langage(cur_die);
      load_file_tabl(cur_die);
      load_basic_type_map(cur_die);
      load_complexe_type_map(cur_die);

      get_die_and_siblings(cur_die);

      dwarf_dealloc(dbg, cur_die, DW_DLA_DIE);
    }
  }

  void DwarfReader::get_die_and_siblings(Dwarf_Die die) {
    Dwarf_Error err = nullptr;
    Dwarf_Die cur_die = die;
    Dwarf_Die sib_die = die;

    Dwarf_Die child = nullptr;
    const int res = dwarf_child(die, &child, nullptr);
    Dwarf_Attribute attr = nullptr;
    std::string type = "notype";
    Dwarf_Off in_type = 0;
    Dwarf_Unsigned in_line = 0;
    Dwarf_Unsigned in_file = 0;

    char* name = nullptr;
    std::string str_name = "(null)";

    Dwarf_Half tag = 0;
    const int got_tag_name = !dwarf_tag(die, &tag, nullptr);

    if (got_tag_name && tag == DW_TAG_subprogram) {
      function info;
      const int got_name = !dwarf_diename(die, &name, nullptr);

      const int got_type = !dwarf_attr(die, DW_AT_type, &attr, nullptr) &&
                           !dwarf_formref(attr, &in_type, nullptr);

      if (got_type) {
        const auto it = type_tabl.find(in_type);
        if (it != type_tabl.end()) type = it->second;
      }

      const int got_line = !dwarf_attr(die, DW_AT_decl_line, &attr, nullptr) &&
                           !dwarf_formudata(attr, &in_line, nullptr);

      const int got_file = !dwarf_attr(die, DW_AT_decl_file, &attr, nullptr) &&
                           !dwarf_formudata(attr, &in_file, &err);

      if (got_name) str_name = std::string(name);
      info.name = std::string(str_name);
      if (got_file) info.file = file_tabl[in_file];
      else
        info.file = std::string("(null)");
      info.retour.type = type;
      info.line = in_line;

      if (res == DW_DLV_OK) parse_function(child, info);
      SYMBOLS_TABL.funs.push_back(info);
    } else if (tag == DW_TAG_variable) {
      variable info;
      const int got_name = !dwarf_diename(die, &name, nullptr);

      const int got_type = !dwarf_attr(die, DW_AT_type, &attr, nullptr) &&
                           !dwarf_formref(attr, &in_type, nullptr);

      if (got_type) {
        const auto it = type_tabl.find(in_type);
        if (it != type_tabl.end()) type = it->second;
      }

      const int got_line = !dwarf_attr(die, DW_AT_decl_line, &attr, nullptr) &&
                           !dwarf_formudata(attr, &in_line, nullptr);

      if (got_name) str_name = std::string(name);
      info.name = std::string(name);
      info.type = type;
      info.line = in_line;
      SYMBOLS_TABL.vars.push_back(info);
    } else if (res == DW_DLV_OK) {
      get_die_and_siblings(child);
      cur_die = child;
      while (dwarf_siblingof(dbg, cur_die, &sib_die, nullptr) == DW_DLV_OK) {
        cur_die = sib_die;
        get_die_and_siblings(sib_die);
      }
    }
  }

  void DwarfReader::load_langage(Dwarf_Die die) {
    Dwarf_Unsigned langage = 0;
    dwarf_srclang(die, &langage, nullptr);

    // 1 2 12 29
    // 4 25 26 33
    if (langage == 1 || langage == 2 || langage == 12 || langage == 29) {
      langsrc = LANGAGE::C;
    } else if (langage == 4 || langage == 25 || langage == 26 || langage == 33) {
      langsrc = LANGAGE::CPP;
    } else {
      langsrc = LANGAGE::UNKNOWN;
    }
  }

  void DwarfReader::load_file_tabl(Dwarf_Die cu_die) {
    char** string;
    Dwarf_Signed size = 0;
    dwarf_srcfiles(cu_die, &string, &size, nullptr);

    for (int i = 0; i < size; i++) { file_tabl.push_back(string[i]); }
  }

  void DwarfReader::load_basic_type_map(Dwarf_Die die) {
    Dwarf_Die cur_die = die;

    Dwarf_Die child = nullptr;
    const int res = dwarf_child(cur_die, &child, nullptr);
    if (res != DW_DLV_OK) return;

    Dwarf_Half tag = 0;
    while (dwarf_siblingof(dbg, child, &cur_die, nullptr) == DW_DLV_OK) {
      dwarf_tag(cur_die, &tag, nullptr);
      if (tag == DW_TAG_base_type || tag == DW_TAG_structure_type || tag == DW_TAG_typedef ||
          tag == DW_TAG_enumeration_type) {

        Dwarf_Off offset = 0;
        dwarf_die_CU_offset(cur_die, &offset, nullptr);
        char* name = nullptr;
        const int got_name = !dwarf_diename(cur_die, &name, nullptr);
        std::string str_name = "(null)";
        if (got_name) str_name = std::string(name);
        type_tabl.insert({offset, str_name});
      }
      child = cur_die;
    }
  }

  void DwarfReader::load_complexe_type_map(Dwarf_Die die) {
    bool have_new_type = true;

    Dwarf_Die die_child = nullptr;
    const int res = dwarf_child(die, &die_child, nullptr);
    if (res != DW_DLV_OK) return;

    while (have_new_type) {
      have_new_type = false;

      Dwarf_Die cur_die = die_child;
      Dwarf_Die cur_child = nullptr;
      while (dwarf_siblingof(dbg, cur_die, &cur_child, nullptr) == DW_DLV_OK) {
        Dwarf_Half tag = 0;
        dwarf_tag(cur_child, &tag, nullptr);

        Dwarf_Off offset = 0;
        std::string name = "null";
        Dwarf_Attribute attr = nullptr;
        if (tag == DW_TAG_pointer_type || tag == DW_TAG_const_type) {
          dwarf_die_CU_offset(cur_child, &offset, nullptr);

          Dwarf_Off in_type = 0;
          const int got_type = !dwarf_attr(cur_child, DW_AT_type, &attr, nullptr) &&
                               !dwarf_formref(attr, &in_type, nullptr);

          if (got_type) {
            const auto it = type_tabl.find(in_type);
            if (it != type_tabl.end()) {
              switch (tag) {
                case DW_TAG_pointer_type:
                  name = it->second + "*";
                  break;
                case DW_TAG_const_type:
                  name = "const " + it->second;
                  break;
                default:
                  break;
              }
              type_tabl.insert({offset, name});
            }
          }
        }
        cur_die = cur_child;
      }
    }
  }

  void DwarfReader::parse_function(Dwarf_Die die, function& info) {
    Dwarf_Half tag = 0;
    dwarf_tag(die, &tag, nullptr);

    char* name = nullptr;
    const int got_name = !dwarf_diename(die, &name, nullptr);
    std::string str_name = "(null)";
    if (got_name) { str_name = std::string(name); }

    Dwarf_Attribute attr;
    Dwarf_Off in_type = 0;
    int got_type =
            !dwarf_attr(die, DW_AT_type, &attr, nullptr) && !dwarf_formref(attr, &in_type, nullptr);

    std::string type_name = "noType";
    if (got_type) {
      auto it = type_tabl.find(in_type);
      if (it != type_tabl.end()) type_name = it->second;
    }

    if (tag == DW_TAG_formal_parameter) {
      info.arg.push_back(param_variable(str_name, type_name));
    } else if (tag == DW_TAG_variable) {
      info.var.push_back(variable(str_name, type_name));
    }

    Dwarf_Die child = 0;
    int res = dwarf_siblingof(dbg, die, &child, nullptr);
    if (res == DW_DLV_OK) parse_function(child, info);
  }

}// namespace ldb
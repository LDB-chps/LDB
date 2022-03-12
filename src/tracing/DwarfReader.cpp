#include "DwarfReader.h"


namespace ldb {

  DwarfReader::DwarfReader(const int fd) {
    int fdd = open("/home/johnkyky/Documents/elf/pid", O_RDONLY);
    if (fdd == -1) {
      std::cout << "error" << std::endl;
      exit(100);
    }

    int res = dwarf_init(fdd, DW_DLC_READ, nullptr, nullptr, &dbg, nullptr);
    if (res == DW_DLV_ERROR) { throw std::runtime_error("dwarf_init failed"); }
  }

  void DwarfReader::populateDwarf(const SymbolTable& symTab) {
    if (!dbg) { throw std::runtime_error("dwarf dbg not initialized"); }

    read_cu(symTab);

    std::cout << "\n\n\n" << std::endl;
    for (auto& file : file_tabl) { std::cout << file << std::endl; }

    std::cout << "\n\n" << std::endl;
    for (auto& i : type_tabl) std::cout << std::hex << i.first << " " << i.second << std::endl;

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
      std::cout << std::endl;
    }
    for (int i = 0; i < SYMBOLS_TABL.vars.size(); i++) {
      std::cout << "L." << SYMBOLS_TABL.vars[i].line << " var : (" << SYMBOLS_TABL.vars[i].type << ") " << SYMBOLS_TABL.vars[i].name
                << std::endl;
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

      load_file_tabl(cur_die);

      load_basic_type_map(cur_die);
      load_complexe_type_map(cur_die);

      Dwarf_Unsigned langage = 0;
      dwarf_srclang(cur_die, &langage, nullptr);
      const char *langage_name = nullptr;
      dwarf_get_LANG_name(langage, &langage_name);
      printf("langage : %d %s", langage, langage_name);

      get_die_and_siblings(cur_die);

      dwarf_dealloc(dbg, cur_die, DW_DLA_DIE);
    }
  }

  void DwarfReader::get_die_and_siblings(Dwarf_Die die) {
    Dwarf_Die cur_die = die;
    Dwarf_Die sib_die = die;

    Dwarf_Die child = nullptr;
    int res = dwarf_child(die, &child, nullptr);

    Dwarf_Half tag = 0;
    int got_tag_name = !dwarf_tag(die, &tag, nullptr);
    if (tag == DW_TAG_subprogram) {
      function info;
      char* name = nullptr;
      dwarf_diename(die, &name, nullptr);

      std::string type = "notype";
      Dwarf_Attribute attr;
      Dwarf_Off in_type = 0;
      int got_type = !dwarf_attr(die, DW_AT_type, &attr, nullptr) &&
                     !dwarf_formref(attr, &in_type, nullptr);

      if (got_type) {
        auto it = type_tabl.find(in_type);
        if (it != type_tabl.end()) type = it->second;
      }

      Dwarf_Off in_line = 0;
      int got_line = !dwarf_attr(die, DW_AT_decl_line, &attr, nullptr) &&
                     !dwarf_formudata(attr, &in_line, nullptr);

      info.name = std::string(name);
      info.retour.type = type;
      info.line = in_line;

      if (res == DW_DLV_OK) parse_function(child, info);
      SYMBOLS_TABL.funs.push_back(info);
    } else if (tag == DW_TAG_variable) {
      variable info;
      char* name = nullptr;
      std::string type = "notype";

      Dwarf_Attribute attr;
      Dwarf_Off in_type = 0;
      int got_type = !dwarf_attr(die, DW_AT_type, &attr, nullptr) &&
                     !dwarf_formref(attr, &in_type, nullptr);

      if (got_type) {
        auto it = type_tabl.find(in_type);
        if (it != type_tabl.end()) type = it->second;
      }

      dwarf_diename(die, &name, nullptr);

      Dwarf_Off in_line = 0;
      int got_line = !dwarf_attr(die, DW_AT_decl_line, &attr, nullptr) &&
                     !dwarf_formudata(attr, &in_line, nullptr);

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

  void DwarfReader::load_file_tabl(Dwarf_Die cu_die) {
    char** string;
    Dwarf_Signed size = 0;
    dwarf_srcfiles(cu_die, &string, &size, nullptr);

    for (int i = 0; i < size; i++) { file_tabl.push_back(string[i]); }
  }

  void DwarfReader::load_basic_type_map(Dwarf_Die die) {
    Dwarf_Die cur_die = die;

    Dwarf_Die child = nullptr;
    int res = dwarf_child(cur_die, &child, nullptr);
    if (res != DW_DLV_OK) return;

    Dwarf_Half tag = 0;
    while (dwarf_siblingof(dbg, child, &cur_die, nullptr) == DW_DLV_OK) {
      dwarf_tag(cur_die, &tag, nullptr);
      if (tag == DW_TAG_base_type || tag == DW_TAG_structure_type || tag == DW_TAG_typedef ||
          tag == DW_TAG_enumeration_type) {
        Dwarf_Off offset = 0;
        dwarf_die_CU_offset(cur_die, &offset, nullptr);
        char* name = nullptr;
        dwarf_diename(cur_die, &name, nullptr);
        type_tabl.insert({offset, std::string(name)});
      }
      child = cur_die;
    }
  }

  void DwarfReader::load_complexe_type_map(Dwarf_Die die) {
    bool is_finish = false;

    Dwarf_Die die_child = nullptr;
    int res = dwarf_child(die, &die_child, nullptr);
    if (res != DW_DLV_OK) return;

    while (not is_finish) {
      is_finish = true;
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
          int got_type = !dwarf_attr(cur_child, DW_AT_type, &attr, nullptr) &&
                         !dwarf_formref(attr, &in_type, nullptr);

          if (got_type) {
            auto it = type_tabl.find(in_type);
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
            } else {
              is_finish = false;
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
    dwarf_diename(die, &name, nullptr);
    std::string str_name;
    if (name) { str_name = std::string(name); }

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

  /*------------------------------------------------------------------------------------------*/

  void populateDwarf(int fd, SymbolTable& symTab) {
    Dwarf_Debug dbg;
    Dwarf_Error err;
    Dwarf_Handler errhand = nullptr;
    Dwarf_Ptr errarg = nullptr;


    fd = open("/home/johnkyky/Documents/elf/pid", O_RDONLY);
    if (fd == -1) {
      std::cout << "error" << std::endl;
      exit(100);
    }

    int res = dwarf_init(fd, DW_DLC_READ, errhand, errarg, &dbg, &err);
    if (res == DW_DLV_NO_ENTRY) {
      return;
    } else if (res == DW_DLV_ERROR) {
      throw std::runtime_error("dwarf_init failed");
    }

    read_cu(dbg);

    for (int i = 0; i < SYMBOLS_TABL.funs.size(); i++) {
      std::cout << SYMBOLS_TABL.funs[i].retour.type << " " << SYMBOLS_TABL.funs[i].name << " ";
      for (int j = 0; j < SYMBOLS_TABL.funs[i].arg.size(); j++)
        std::cout << "arg" << j << "(" << SYMBOLS_TABL.funs[i].arg[j].type << " "
                  << SYMBOLS_TABL.funs[i].arg[j].name << ") ";
      for (int j = 0; j < SYMBOLS_TABL.funs[i].var.size(); j++)
        std::cout << "var" << j << "(" << SYMBOLS_TABL.funs[i].var[j].type << " "
                  << SYMBOLS_TABL.funs[i].var[j].name << ") ";
      std::cout << std::endl;
    }
    for (int i = 0; i < SYMBOLS_TABL.vars.size(); i++) {
      std::cout << "var : (" << SYMBOLS_TABL.vars[i].type << ") " << SYMBOLS_TABL.vars[i].name
                << std::endl;
    }


    dwarf_finish(dbg, &err);
  }


  void read_cu(Dwarf_Debug dbg) {
    Dwarf_Unsigned cu_header_length = 0;
    Dwarf_Half version_stamp = 0;
    Dwarf_Unsigned abbrev_offset = 0;
    Dwarf_Half address_size = 0;
    Dwarf_Unsigned next_cu_header = 0;
    Dwarf_Error error;
    bool done = true;

    while (done) {
      int res = dwarf_next_cu_header(dbg, &cu_header_length, &version_stamp, &abbrev_offset,
                                     &address_size, &next_cu_header, &error);

      if (res == DW_DLV_NO_ENTRY) {
        return;
      } else if (res == DW_DLV_ERROR) {
        throw std::runtime_error("read_cu failed");
      }

      Dwarf_Die cu_die = nullptr;
      res = dwarf_siblingof(dbg, nullptr, &cu_die, &error);
      if (res == DW_DLV_ERROR) { throw std::runtime_error("dwarf_siblingof failed"); }
      if (res == DW_DLV_NO_ENTRY) { throw std::runtime_error("No DIEs in CU"); }

      auto file_table = read_file_tabl(cu_die);
      std::cout << "\n\nfile_table: " << std::endl;
      for (auto& file : file_table) { std::cout << file << std::endl; }

      load_basic_type_map(dbg, cu_die);
      load_pointer_type_map(dbg, cu_die);

      for (auto& i : TYPE_TABL) std::cout << std::hex << i.first << " " << i.second << std::endl;
      printf("\n\n\n");
      get_die_and_siblings(dbg, cu_die, 0);

      dwarf_dealloc(dbg, cu_die, DW_DLA_DIE);
    }
  }


  std::vector<std::string> read_file_tabl(Dwarf_Die cu_die) {
    std::vector<std::string> res;

    Dwarf_Error err = nullptr;
    char** string;
    Dwarf_Signed size = 0;
    dwarf_srcfiles(cu_die, &string, &size, &err);

    for (int i = 0; i < size; i++) { res.push_back(string[i]); }

    return res;
  }


  void load_basic_type_map(Dwarf_Debug dbg, Dwarf_Die die) {
    int res = DW_DLV_ERROR;
    Dwarf_Die cur_die = die;
    Dwarf_Die sib_die = die;

    Dwarf_Die child = 0;
    res = dwarf_child(cur_die, &child, nullptr);
    if (res != DW_DLV_OK) return;

    Dwarf_Half tag = 0;
    while (dwarf_siblingof(dbg, child, &sib_die, nullptr) == DW_DLV_OK) {
      int got_tag_name = !dwarf_tag(sib_die, &tag, nullptr);
      if (tag == DW_TAG_base_type || tag == DW_TAG_structure_type || tag == DW_TAG_typedef) {
        Dwarf_Off offset = 0;
        dwarf_die_CU_offset(sib_die, &offset, nullptr);
        print_die_data(dbg, sib_die, 0);
        char* name = nullptr;
        dwarf_diename(sib_die, &name, nullptr);
        TYPE_TABL.insert({offset, std::string(name)});
      }
      child = sib_die;
    }
  }


  void load_pointer_type_map(Dwarf_Debug dbg, Dwarf_Die die) {
    bool is_finish = false;
    int res = DW_DLV_ERROR;
    Dwarf_Die cur_die = die;
    Dwarf_Die sib_die = die;

    Dwarf_Die child = 0;
    res = dwarf_child(cur_die, &child, nullptr);
    if (res != DW_DLV_OK) return;

    Dwarf_Half tag = 0;
    while (not is_finish) {
      is_finish = true;
      Dwarf_Die cu_die = child;
      while (dwarf_siblingof(dbg, cu_die, &sib_die, nullptr) == DW_DLV_OK) {
        int got_tag_name = !dwarf_tag(sib_die, &tag, nullptr);

        if (tag == DW_TAG_pointer_type) {
          Dwarf_Off offset = 0;
          dwarf_die_CU_offset(sib_die, &offset, nullptr);

          std::string name = "null";
          Dwarf_Attribute attr;
          Dwarf_Off in_type = 0;
          int got_type = !dwarf_attr(sib_die, DW_AT_type, &attr, nullptr) &&
                         !dwarf_formref(attr, &in_type, nullptr);

          if (got_type) {
            auto it = TYPE_TABL.find(in_type);
            if (it != TYPE_TABL.end()) name = "*" + it->second;
            TYPE_TABL.insert({offset, name});
          } else {
            is_finish = false;
          }
        }
        cu_die = sib_die;
      }
    }
  }


  void get_die_and_siblings(Dwarf_Debug dbg, Dwarf_Die die, int level) {
    int res = DW_DLV_ERROR;
    Dwarf_Die cur_die = die;
    Dwarf_Die sib_die = die;
    Dwarf_Error err;

    print_die_data(dbg, die, level);

    Dwarf_Die child = 0;
    res = dwarf_child(cur_die, &child, &err);


    Dwarf_Half tag = 0;
    int got_tag_name = !dwarf_tag(die, &tag, &err);
    if (tag == DW_TAG_subprogram && 1) {
      function info;
      char* name = nullptr;
      dwarf_diename(die, &name, nullptr);
      std::string type = "notype";

      Dwarf_Attribute attr;
      Dwarf_Off in_type = 0;
      int got_type =
              !dwarf_attr(die, DW_AT_type, &attr, &err) && !dwarf_formref(attr, &in_type, &err);

      if (got_type) {
        auto it = TYPE_TABL.find(in_type);
        if (it != TYPE_TABL.end()) type = it->second;
      }
      info.name = std::string(name);
      info.retour.type = type;

      if (res == DW_DLV_OK) parse_function(dbg, child, info);
      SYMBOLS_TABL.funs.push_back(info);
    } else if (tag == DW_TAG_variable && 1) {
      variable info;
      char* name = nullptr;
      std::string type = "notype";

      Dwarf_Attribute attr;
      Dwarf_Off in_type = 0;
      int got_type =
              !dwarf_attr(die, DW_AT_type, &attr, &err) && !dwarf_formref(attr, &in_type, &err);

      if (got_type) {
        auto it = TYPE_TABL.find(in_type);
        if (it != TYPE_TABL.end()) type = it->second;
      }

      dwarf_diename(die, &name, nullptr);
      info.name = std::string(name);
      info.type = type;
      SYMBOLS_TABL.vars.push_back(info);
    } else if (res == DW_DLV_OK) {
      get_die_and_siblings(dbg, child, level + 1);
      cur_die = child;
      while (dwarf_siblingof(dbg, cur_die, &sib_die, &err) == DW_DLV_OK) {
        cur_die = sib_die;
        get_die_and_siblings(dbg, sib_die, level + 1);
      }
    }
  }


  void parse_function(Dwarf_Debug dbg, Dwarf_Die die, function& info) {
    Dwarf_Die child = 0;
    int res = dwarf_siblingof(dbg, die, &child, nullptr);


    Dwarf_Half tag = 0;
    dwarf_tag(die, &tag, nullptr);

    char* name = nullptr;
    dwarf_diename(die, &name, nullptr);

    if (name) {
      std::string type = "notype";

      Dwarf_Attribute attr;
      Dwarf_Off in_type = 0;
      int got_type = !dwarf_attr(die, DW_AT_type, &attr, nullptr) &&
                     !dwarf_formref(attr, &in_type, nullptr);

      if (got_type) {
        auto it = TYPE_TABL.find(in_type);
        if (it != TYPE_TABL.end()) type = it->second;
      }
      if (tag == DW_TAG_formal_parameter) {
        info.arg.push_back(param_variable(name, type));
      } else if (tag == DW_TAG_variable) {
        info.var.push_back(variable(name, type));
      }
    }

    if (res == DW_DLV_OK) parse_function(dbg, child, info);
  }


  void print_die_data(Dwarf_Debug dbg, Dwarf_Die die, int level) {

    Dwarf_Error err = nullptr;

    Dwarf_Half tag = 0;
    char* name = nullptr;
    const char* tag_name = nullptr;

    int got_name = !dwarf_diename(die, &name, &err);
    int got_tag_name = !dwarf_tag(die, &tag, &err) && dwarf_get_TAG_name(tag, &tag_name);

    Dwarf_Attribute attr;
    Dwarf_Off in_type = 0;
    int got_type =
            !dwarf_attr(die, DW_AT_type, &attr, &err) && !dwarf_formref(attr, &in_type, &err);

    printf("%2d name %30s %30s %5x\n", level, name, tag_name, in_type);
  }

}// namespace ldb
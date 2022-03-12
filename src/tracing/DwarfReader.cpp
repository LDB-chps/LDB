#include "DwarfReader.h"


namespace ldb {

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
      std::cout << SYMBOLS_TABL.funs[i].name << " ";
      for (int j = 0; j < SYMBOLS_TABL.funs[i].arg.size(); j++)
        std::cout << "arg" << j << "(" << SYMBOLS_TABL.funs[i].arg[j].name << ") ";
      for (int j = 0; j < SYMBOLS_TABL.funs[i].var.size(); j++)
        std::cout << "var" << j << "(" << SYMBOLS_TABL.funs[i].var[j].name << ") ";
      std::cout << std::endl;
    }
    for (int i = 0; i < SYMBOLS_TABL.vars.size(); i++) {
      std::cout << "var : " << SYMBOLS_TABL.vars[i].name << std::endl;
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

      load_type_map(dbg, cu_die);

      for (auto& i : TYPE_TABL) std::cout << i.first << " " << i.second << std::endl;
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


  void load_type_map(Dwarf_Debug dbg, Dwarf_Die die) {
    int res = DW_DLV_ERROR;
    Dwarf_Die cur_die = die;
    Dwarf_Die sib_die = die;

    Dwarf_Die child = 0;
    res = dwarf_child(cur_die, &child, nullptr);
    if (res != DW_DLV_OK) return;

    Dwarf_Half tag = 0;
    while (dwarf_siblingof(dbg, child, &sib_die, nullptr) == DW_DLV_OK) {
      int got_tag_name = !dwarf_tag(sib_die, &tag, nullptr);
      if (tag == DW_TAG_base_type || tag == DW_TAG_structure_type) {
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


  void get_die_and_siblings(Dwarf_Debug dbg, Dwarf_Die die, int level) {
    int res = DW_DLV_ERROR;
    Dwarf_Die cur_die = die;
    Dwarf_Die sib_die = die;
    Dwarf_Error err;

    // Dwarf_Off offset = 0;
    // dwarf_die_CU_offset(cur_die, &offset, &err);
    // printf("CU at offset 0x%8.8x:\n", offset);

    print_die_data(dbg, die, level);

    Dwarf_Die child = 0;
    res = dwarf_child(cur_die, &child, &err);


    Dwarf_Half tag = 0;
    int got_tag_name = !dwarf_tag(die, &tag, &err);
    if (tag == DW_TAG_subprogram && 0) {
      function info;
      char* name = nullptr;
      dwarf_diename(die, &name, nullptr);
      info.name = std::string(name);

      if (res == DW_DLV_OK) parse_function(dbg, child, info);

      SYMBOLS_TABL.funs.push_back(info);
    } else if (tag == DW_TAG_variable && 0) {
      variable info;
      char* name = nullptr;
      dwarf_diename(die, &name, nullptr);
      info.name = std::string(name);
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
      std::stringstream ss;
      if (tag == DW_TAG_formal_parameter) {
        info.arg.push_back(param_variable(name, ""));
      } else if (tag == DW_TAG_variable) {
        info.var.push_back(variable(name, ""));
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

    printf("%2d name %30s %30s %d\n", level, name, tag_name, tag);
  }

}// namespace ldb
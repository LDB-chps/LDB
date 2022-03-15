#include "DwarfReader.h"


namespace ldb {

  enum class LANGAGE { C, CPP, UNKNOWN };

  class DwarfReader {
  public:
    DwarfReader(Elf* elf);
    ~DwarfReader() = default;

    void populateDwarf(SymbolTable& symTab);

    const LANGAGE getLangage() {
      return langsrc;
    }

  private:
    void readCu(SymbolTable& symTab);
    void getDieAndSiblings(const Dwarf_Die die, SymbolTable& symTable);

    void loadLangage(Dwarf_Die die);
    void loadFileTable(Dwarf_Die die);
    void loadBasicTypeMap(Dwarf_Die die);
    void loadComplexeTypeMap(Dwarf_Die die);
    void parseFunction(Dwarf_Die die, Symbol& fun);

  private:
    Dwarf_Debug dbg;
    LANGAGE langsrc;
    std::vector<std::string> file_tabl;
    std::map<Dwarf_Off, std::string> type_tabl;
  };


  DwarfReader::DwarfReader(Elf* elf) : langsrc(LANGAGE::UNKNOWN) {
    const int res = dwarf_elf_init(elf, DW_DLC_READ, nullptr, nullptr, &dbg, nullptr);
    if (res == DW_DLV_ERROR) { throw std::runtime_error("dwarf_init failed"); }
  }

  void DwarfReader::populateDwarf(SymbolTable& symTab) {
    if (!dbg) { throw std::runtime_error("dwarf dbg not initialized"); }

    readCu(symTab);
  }

  void DwarfReader::readCu(SymbolTable& symTable) {
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

      loadLangage(cur_die);
      loadFileTable(cur_die);
      loadBasicTypeMap(cur_die);
      loadComplexeTypeMap(cur_die);

      getDieAndSiblings(cur_die, symTable);

      dwarf_dealloc(dbg, cur_die, DW_DLA_DIE);
      cur_die = nullptr;
    }
  }

  void DwarfReader::getDieAndSiblings(Dwarf_Die die, SymbolTable& symTable) {
    Dwarf_Error err = nullptr;
    Dwarf_Die cur_die = die;
    Dwarf_Die sib_die = die;

    Dwarf_Die child = nullptr;
    const int res = dwarf_child(die, &child, nullptr);
    Dwarf_Attribute attr = nullptr;
    std::string type = "";
    Dwarf_Off in_type = 0;
    Dwarf_Unsigned in_line = 0;
    Dwarf_Unsigned in_file = 0;

    char* name = nullptr;
    std::string str_name = "";

    Dwarf_Half tag = 0;
    const int got_tag_name = !dwarf_tag(die, &tag, nullptr);

    if (got_tag_name && tag == DW_TAG_subprogram) {
      Dwarf_Bool in_declaration = 0;
      const int got_declaration = !dwarf_attr(die, DW_AT_declaration, &attr, nullptr) &&
                                  !dwarf_formflag(attr, &in_declaration, nullptr);

      if (in_declaration) return;

      const int got_name = !dwarf_diename(die, &name, &err);

      const int got_type =
              !dwarf_attr(die, DW_AT_type, &attr, nullptr) && !dwarf_formref(attr, &in_type, &err);

      if (got_type) {
        const auto it = type_tabl.find(in_type);
        if (it != type_tabl.end()) type = it->second;
      }

      const int got_line = !dwarf_attr(die, DW_AT_decl_line, &attr, nullptr) &&
                           !dwarf_formudata(attr, &in_line, &err);

      const int got_file = !dwarf_attr(die, DW_AT_decl_file, &attr, nullptr) &&
                           !dwarf_formudata(attr, &in_file, &err);

      if (got_name) {
        str_name = std::string(name);
        dwarf_dealloc(dbg, name, DW_DLA_STRING);
        name = nullptr;
      }

      auto* fun = symTable[str_name];

      if (!fun) {
        return;
      }

      fun->setLine(in_line);
      if (got_file) fun->setFile(file_tabl[in_file]);
      if (got_type) fun->setType(type);

      if (res == DW_DLV_OK) parseFunction(child, *fun);
    } /* else if (tag == DW_TAG_variable) {
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
     }*/
    else if (res == DW_DLV_OK) {
      getDieAndSiblings(child, symTable);
      cur_die = child;
      while (dwarf_siblingof(dbg, cur_die, &sib_die, nullptr) == DW_DLV_OK) {
        dwarf_dealloc(dbg, cur_die, DW_DLA_DIE);
        cur_die = nullptr;
        cur_die = sib_die;
        getDieAndSiblings(sib_die, symTable);
      }
      dwarf_dealloc(dbg, cur_die, DW_DLA_DIE);
      cur_die = nullptr;
      dwarf_dealloc(dbg, child, DW_DLA_DIE);
      child = nullptr;
    }
  }

  void DwarfReader::loadLangage(Dwarf_Die die) {
    Dwarf_Unsigned langage = 0;
    dwarf_srclang(die, &langage, nullptr);

    if (langage == 1 || langage == 2 || langage == 12 || langage == 29) {
      langsrc = LANGAGE::C;
    } else if (langage == 4 || langage == 25 || langage == 26 || langage == 33) {
      langsrc = LANGAGE::CPP;
    } else {
      langsrc = LANGAGE::UNKNOWN;
    }
  }

  void DwarfReader::loadFileTable(Dwarf_Die cu_die) {
    char** string;
    Dwarf_Signed size = 0;
    dwarf_srcfiles(cu_die, &string, &size, nullptr);

    for (int i = 0; i < size; i++) {
      file_tabl.push_back(string[i]);
      dwarf_dealloc(dbg, string[i], DW_DLA_STRING);
      string[i] = nullptr;
    }
    dwarf_dealloc(dbg, string, DW_DLA_STRING);
    string = nullptr;
  }

  void DwarfReader::loadBasicTypeMap(Dwarf_Die die) {
    Dwarf_Die child = nullptr;
    const int res = dwarf_child(die, &child, nullptr);
    if (res != DW_DLV_OK) return;

    Dwarf_Die cur_die = die;
    while (dwarf_siblingof(dbg, child, &cur_die, nullptr) == DW_DLV_OK) {
      Dwarf_Half tag = 0;
      dwarf_tag(cur_die, &tag, nullptr);
      if (tag == DW_TAG_base_type || tag == DW_TAG_structure_type || tag == DW_TAG_typedef ||
          tag == DW_TAG_enumeration_type) {

        Dwarf_Off offset = 0;
        dwarf_die_CU_offset(cur_die, &offset, nullptr);
        char* name = nullptr;
        const int got_name = !dwarf_diename(cur_die, &name, nullptr);
        std::string str_name = "";
        if (got_name) str_name = std::string(name);
        dwarf_dealloc(dbg, name, DW_DLA_STRING);
        name = nullptr;
        type_tabl.insert({offset, str_name});
      }
      dwarf_dealloc(dbg, child, DW_DLA_DIE);
      child = nullptr;
      child = cur_die;
    }
    if (cur_die != die) {
      dwarf_dealloc(dbg, cur_die, DW_DLA_DIE);
      cur_die = nullptr;
    }
  }

  void DwarfReader::loadComplexeTypeMap(Dwarf_Die die) {
    bool have_new_type = true;

    Dwarf_Error err = nullptr;
    Dwarf_Die die_child = nullptr;
    const int res = dwarf_child(die, &die_child, nullptr);
    if (res != DW_DLV_OK) return;

    while (have_new_type) {
      have_new_type = false;

      Dwarf_Die cur_die = die_child;
      Dwarf_Die cur_child = nullptr;
      while (dwarf_siblingof(dbg, cur_die, &cur_child, nullptr) == DW_DLV_OK) {
        dwarf_dealloc(dbg, cur_die, DW_DLA_DIE);
        cur_die = nullptr;
        Dwarf_Half tag = 0;
        dwarf_tag(cur_child, &tag, nullptr);

        Dwarf_Off offset = 0;
        std::string name = "";
        Dwarf_Attribute attr = nullptr;
        if (tag == DW_TAG_pointer_type || tag == DW_TAG_const_type) {
          dwarf_die_CU_offset(cur_child, &offset, nullptr);

          Dwarf_Off in_type = 0;
          const int got_type = !dwarf_attr(cur_child, DW_AT_type, &attr, nullptr) &&
                               !dwarf_formref(attr, &in_type, &err);

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
      dwarf_dealloc(dbg, cur_die, DW_DLA_DIE);
      cur_die = nullptr;
    }
    dwarf_dealloc(dbg, die_child, DW_DLA_DIE);
    die_child = nullptr;
  }

  void DwarfReader::parseFunction(Dwarf_Die die, Symbol& fun) {
    Dwarf_Error err = nullptr;
    Dwarf_Half tag = 0;
    dwarf_tag(die, &tag, nullptr);

    char* name = nullptr;
    const int got_name = !dwarf_diename(die, &name, nullptr);
    std::string str_name = "";
    if (got_name) {
      str_name = std::string(name);
      dwarf_dealloc(dbg, name, DW_DLA_STRING);
      name = nullptr;
    }

    Dwarf_Attribute attr;
    Dwarf_Off in_type = 0;
    int got_type =
            !dwarf_attr(die, DW_AT_type, &attr, nullptr) && !dwarf_formref(attr, &in_type, &err);

    std::string type_name = "";
    if (got_type) {
      auto it = type_tabl.find(in_type);
      if (it != type_tabl.end()) type_name = it->second;
    }

    if (tag == DW_TAG_formal_parameter) {
      fun.getArgs().push_back(Symbol(str_name, type_name));
    } else if (tag == DW_TAG_variable) {
      fun.getVars().push_back(Symbol(str_name, type_name));
    }

    Dwarf_Die child = 0;
    int res = dwarf_siblingof(dbg, die, &child, nullptr);
    dwarf_dealloc(dbg, die, DW_DLA_DIE);
    die = nullptr;
    if (res == DW_DLV_OK) parseFunction(child, fun);
  }

  void readDwarfDebugInfo(Elf* elf, DebugInfo& db) {
    DwarfReader reader(elf);
    reader.populateDwarf(*db.getSymbolTable());
  }

}// namespace ldb
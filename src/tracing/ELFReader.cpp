#include "ELFReader.h"


namespace ldb {

  std::unique_ptr<ELFFile> ELFReader::read(const std::filesystem::path& path, pid_t pid) {
    std::unique_ptr<ELFFile> elfFile(new ELFFile(path));
    std::ifstream file(path, std::ios::binary);
    auto header = parseHeader(file);

    if (not header) return nullptr;

    auto string_table = parseStringTable(file, *header);
    if (not string_table) return nullptr;

    auto section = parseSections(file, *header, *string_table);
    auto symbol_list = parseSymbols(file, *header, *string_table);

    elfFile->sections = std::move(section);
    elfFile->symbols = std::move(symbol_list);

    return elfFile;
  }

  std::optional<Elf64_Ehdr> parseHeader(std::istream& stream) {
    stream.seekg(0);

    Elf64_Ehdr header;
    if (not stream.read(reinterpret_cast<char*>(&header), sizeof(header))) { return std::nullopt; }

    return header;
  }

  std::optional<std::string> ELFReader::parseStringTable(std::istream& istream, Elf64_Ehdr header) {
    istream.seekg(header.e_shoff + header.e_shstrndx * header.e_shentsize);

    Elf64_Shdr section_header;
    if (not istream.read(reinterpret_cast<char*>(&section_header), sizeof(Elf64_Shdr))) {
      throw std::runtime_error("Failed to read section header");
    }

    std::string string_table;
    string_table.resize(section_header.sh_size);

    if (not istream.seekg(section_header.sh_offset) or
        not istream.read(string_table.data(), section_header.sh_size) or string_table[0] != '\0') {
      throw std::runtime_error("Failed to read string table");
    }
    return string_table;
  }


  std::vector<Section> parseSections(std::istream& stream, const Elf64_Ehdr& header,
                                     const std::string& string_table) {
    std::vector<Section> sections;
    sections.reserve(header.e_shnum);
    stream.seekg(header.e_shoff);

    for (size_t i = 0; i < header.e_shnum; ++i) {
      Elf64_Shdr section_header;
      if (not stream.read(reinterpret_cast<char*>(&section_header), sizeof(Elf64_Shdr))) {
        throw std::runtime_error("Failed to read section header");
      }
      std::string name = std::string(string_table.c_str() + section_header.sh_name);
      SectionType type = SectionType::kUnknown;

      sections.emplace_back(type, name, section_header.sh_offset, section_header.sh_addr,
                            section_header.sh_size);
    }
    return sections;
  }

  std::shared_ptr<SymbolList> parseSymbols(std::istream& stream, const Elf64_Shdr& sym_section,
                                           const std::string& String_table) {
    auto symbols = std::make_unique<SymbolList>();
    stream.seekg(sym_section.sh_offset);

    for (size_t i = 0; i < sym_section.sh_size / sizeof(Elf64_Sym); i++) {
      Elf64_Sym sym;

      if (not stream.read(reinterpret_cast<char*>(&sym), sizeof(Elf64_Sym))) {
        throw std::runtime_error("Failed to read symbol");
      }

      std::string name = std::string(String_table.c_str() + sym.st_name);
      SymbolType type = SymbolType::kUnknown;
      symbols->emplace_back(type, sym.st_value, name, std::nullopt);
    }
    return symbols;
  }
}// namespace ldb
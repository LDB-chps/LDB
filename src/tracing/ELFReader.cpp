#include "ELFReader.h"
#include <algorithm>
#include <execution>


namespace ldb {

  std::unique_ptr<ELFFile> ELFReader::read(const std::filesystem::path& path, pid_t pid) {
    std::unique_ptr<ELFFile> elfFile(new ELFFile(path));
    std::ifstream file(path, std::ios::binary);
    auto header = parseHeader(file);

    if (not header) return nullptr;

    auto string_table = parseStringTable(file, *header);
    if (not string_table) return nullptr;

    std::vector<Section> sections = parseSections(file, *header, *string_table);

    auto symbol_list = parseSymbols(file, sections);

    elfFile->sections = std::move(sections);
    elfFile->symbols = std::move(symbol_list);

    return elfFile;
  }

  std::optional<Elf64_Ehdr> ELFReader::parseHeader(std::istream& stream) {
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


  std::vector<Section> ELFReader::parseSections(std::istream& stream, const Elf64_Ehdr& header,
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
      auto type = static_cast<SectionType>(section_header.sh_type);

      std::cout << name << " section_offset: " << section_header.sh_offset
                << " hdr_offset: " << stream.tellg() << " vaddr: " << section_header.sh_addr
                << std::endl;

      sections.emplace_back(type, name, i * sizeof(Elf64_Shdr) + header.e_shoff,
                            section_header.sh_offset, section_header.sh_size,
                            section_header.sh_addr);
    }
    return sections;
  }

  std::shared_ptr<SymbolList> ELFReader::parseSymbols(std::istream& stream,
                                                      const std::vector<Section>& sections) {
    auto symbols = std::make_unique<SymbolList>();

    for (auto& sec : sections) {
      if (sec.getType() == SectionType::kSymTab) {
        Elf64_Shdr section_header;
        stream.seekg(sec.getHdrOffset());
        if (not stream.read(reinterpret_cast<char*>(&section_header), sizeof(Elf64_Shdr))) {
          throw std::runtime_error("Failed to read section header");
        }


        // Symbol tables uses a different string table than the other sections
        std::string symbol_str_table;
        symbol_str_table.resize(sections[section_header.sh_link].getSize());
        stream.seekg(sections[section_header.sh_link].getSectionOffset());

        if (not stream.read(symbol_str_table.data(), sections[section_header.sh_link].getSize())) {
          throw std::runtime_error("Failed to read symbol string table");
        }

        // Jump to the beginning of the symbol table
        stream.seekg(sec.getSectionOffset());
        size_t n_sym = sec.getSize() / sizeof(Elf64_Shdr);

        for (size_t i = 0; i < n_sym; i++) {
          Elf64_Sym sym;

          if (not stream.read(reinterpret_cast<char*>(&sym), sizeof(Elf64_Sym))) {
            throw std::runtime_error("Failed to read symbol");
          }

          // if (ELF64_ST_TYPE(sym.st_info) != STT_FUNC) { continue; }

          std::string name = std::string(symbol_str_table.c_str() + sym.st_name);

          symbols->emplace_back(sym.st_value, name, std::nullopt);
        }
      }
    }

    return symbols;
  }
}// namespace ldb
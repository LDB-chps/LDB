#include "ELFReader.h"
#include <algorithm>
#include <boost/asio.hpp>
#include <boost/core/demangle.hpp>
#include <execution>
#include <future>
#include <link.h>
#include <sys/ptrace.h>
#include <sys/wait.h>

namespace ldb {

namespace {
std::optional<Elf64_Addr> locateLinkMap(std::istream &stream,
                                        const std::vector<Section> &sections,
                                        pid_t pid) {
  auto dynsec_it =
      std::find_if(sections.begin(), sections.end(),
                   [](const Section &s) { return s.getName() == ".dynamic"; });

  if (dynsec_it == sections.end())
    return std::nullopt;

  auto &dynsec = *dynsec_it;
  size_t ndyn = dynsec.getSize() / sizeof(Elf64_Dyn);

  std::cout << "Found .dynamic section with " << ndyn << " entries at "
            << std::hex << dynsec.getVirtualAddress() << std::endl;

  Elf64_Dyn dyn;
  for (size_t i = 0; i < ndyn; i++) {
    Elf64_Addr curr_addr = dynsec.getVirtualAddress() + i * sizeof(Elf64_Dyn);
    long val = ptrace(PTRACE_PEEKDATA, pid, curr_addr, nullptr);
    if (val == -1)
      return std::nullopt;
    dyn.d_tag = val;

    if (dyn.d_tag == DT_DEBUG) {
      Elf64_Addr r_debug_addr = (Elf64_Addr)ptrace(
          PTRACE_PEEKDATA, pid, curr_addr + sizeof(dyn.d_tag), 0);
      Elf64_Addr link_map_addr =
          (Elf64_Addr)ptrace(PTRACE_PEEKDATA, pid,
                             r_debug_addr + offsetof(struct r_debug, r_map), 0);

      return link_map_addr;
    }
  }
  return std::nullopt;
}

std::unique_ptr<SymbolTable>
parseDynamicSymbols(std::istream &stream, const std::vector<Section> &sections,
                    pid_t pid) {
  auto dynsec =
      std::find_if(sections.begin(), sections.end(), [](const Section &sec) {
        return sec.getType() == SectionType::kDynSym;
      });
  if (dynsec == sections.end())
    return nullptr;

  auto link_map_addr = locateLinkMap(stream, sections, pid);
  if (not link_map_addr)
    return nullptr;
  std::cout << "Link map address: " << std::hex << *link_map_addr << std::endl;
  return nullptr;
}

} // namespace

std::unique_ptr<DebugInfo> ELFReader::read(const std::filesystem::path &path,
                                           pid_t pid) {
  std::unique_ptr<DebugInfo> res(new DebugInfo());
  std::ifstream file_stream(path, std::ios::binary);

  long tr = ptrace(PTRACE_ATTACH, pid, nullptr, nullptr);
  waitpid(pid, nullptr, 0);

  auto header = parseHeader(file_stream);

  if (not header)
    return nullptr;

  auto string_table = parseStringTable(
      file_stream, header->e_shstrndx * header->e_shentsize + header->e_shoff);
  if (not string_table)
    return nullptr;

  std::vector<Section> sections(
      parseSections(file_stream, *header, *string_table));
  if (sections.empty())
    return nullptr;

  std::unique_ptr<SymbolTable> local_symbol_table =
      parseSymbols(file_stream, sections);
  if (not local_symbol_table)
    return nullptr;

  // std::unique_ptr<SymbolTable> dynamic_symbol_table =
  //         parseDynamicSymbols(file_stream, sections, pid);
  // if (not dynamic_symbol_table) return nullptr;

  // local_symbol_table->join(std::move(dynamic_symbol_table));

  res->symbols_table = std::move(*local_symbol_table);

  // Parse dwarf
   populateDwarf(0, res->symbols_table);

  return res;
}

std::optional<Elf64_Ehdr> ELFReader::parseHeader(std::istream &stream) {
  stream.seekg(0);

  Elf64_Ehdr header;
  if (not stream.read(reinterpret_cast<char *>(&header), sizeof(header))) {
    return std::nullopt;
  }

  return header;
}

std::optional<std::string> ELFReader::parseStringTable(std::istream &istream,
                                                       size_t offset) {
  istream.seekg(offset);

  Elf64_Shdr section_header;
  if (not istream.read(reinterpret_cast<char *>(&section_header),
                       sizeof(Elf64_Shdr))) {
    throw std::runtime_error("Failed to read section header");
  }

  std::string string_table;
  string_table.resize(section_header.sh_size);

  if (not istream.seekg(section_header.sh_offset) or
      not istream.read(string_table.data(), section_header.sh_size) or
      string_table[0] != '\0') {
    return std::nullopt;
  }
  return string_table;
}

std::vector<Section> ELFReader::parseSections(std::istream &stream,
                                              const Elf64_Ehdr &header,
                                              const std::string &string_table) {
  std::vector<Section> sections;
  sections.reserve(header.e_shnum);
  stream.seekg(header.e_shoff);

  for (size_t i = 0; i < header.e_shnum; ++i) {
    Elf64_Shdr section_header;
    if (not stream.read(reinterpret_cast<char *>(&section_header),
                        sizeof(Elf64_Shdr))) {
      return {};
    }
    std::string name =
        std::string(string_table.c_str() + section_header.sh_name);
    auto type = static_cast<SectionType>(section_header.sh_type);

    std::cout << name << " section_offset: " << section_header.sh_offset
              << " hdr_offset: " << stream.tellg()
              << " vaddr: " << section_header.sh_addr << std::endl;

    sections.emplace_back(type, name, i * sizeof(Elf64_Shdr) + header.e_shoff,
                          section_header.sh_offset, section_header.sh_size,
                          section_header.sh_addr);
  }
  return sections;
}

std::unique_ptr<SymbolTable>
ELFReader::parseSymbols(std::istream &stream,
                        const std::vector<Section> &sections) {
  std::unique_ptr<SymbolTable> symbols = nullptr;

  for (auto &sec : sections) {
    if (sec.getType() != SectionType::kSymTab) continue;

    Elf64_Shdr section_header;
    stream.seekg(sec.getHdrOffset());
    if (not stream.read(reinterpret_cast<char *>(&section_header),
                        sizeof(Elf64_Shdr))) {
      return nullptr;
    }

    auto sym_str_table = parseStringTable(
        stream, sections[section_header.sh_link].getHdrOffset());
    if (not sym_str_table) {
      return nullptr;
    }

    // Jump to the beginning of the symbol table
    stream.seekg(sec.getSectionOffset());
    size_t n_sym = sec.getSize() / sizeof(Elf64_Sym);
    auto buff = std::make_unique<SymbolTable>(n_sym);

    for (size_t i = 0; i < n_sym; i++) {
      Elf64_Sym sym;

      if (not stream.read(reinterpret_cast<char *>(&sym), sizeof(Elf64_Sym))) {
        return nullptr;
      }

      // For this project, we will only consider functions
      /*if (ELF64_ST_TYPE(sym.st_info) != STT_FUNC or sym.st_shndx >
      sections.size() or sym.st_shndx == SHN_UNDEF) { continue;
      } */

      // Names might be mangled if we're dealing with cpp
      std::string name =
          boost::core::demangle(sym_str_table->c_str() + sym.st_name);

      buff->emplace_back(sym.st_value, name, std::nullopt);
    }
    buff->join(std::move(symbols));
    symbols = std::move(buff);
  }

  return symbols;
}

} // namespace ldb
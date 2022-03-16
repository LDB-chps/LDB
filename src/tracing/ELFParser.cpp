#include "ELFParser.h"
#include "DwarfReader.h"
#include <algorithm>
#include <boost/asio.hpp>
#include <boost/core/demangle.hpp>
#include <execution>
#include <future>
#include <libdwarf/libdwarf.h>
#include <libelf.h>
#include <link.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <utility>

// Parsing elf / dwarf data proved to be a bit of a pain since there's no official tutorial or such
// except for the Standard specifications.
//
// In the same spirit, both libelf and libdwarf are void of
// documentation except for unofficial OS-specific ones. The following code combines both
// libraries, and is an improvement of https://code.lm7.fr/robotux/pstack.
//
// The following code is not optimized for speed, but rather for readability, since we don't want
// the future reader to suffer as we did.

namespace fs = std::filesystem;

namespace ldb {


  namespace {

    /**
     * @brief Helper class to parse an elf file
     * This class is declared locally since the user should only use the readDebugInfo(...) function
     * This allows the function to be stateless and thus thread-safe.
     */
    class ELFFile {
    public:
      static std::unique_ptr<ELFFile> make(const fs::path& elf_file) {
        auto res = std::unique_ptr<ELFFile>(new ELFFile(elf_file));
        if (res->badbit) {
          std::cerr << "Failed to open " << elf_file << std::endl;
          return nullptr;
        }
        return res;
      }

      ~ELFFile();

      /**
       * @brief This object should not be copyable
       * @param other
       */
      ELFFile(const ELFFile& other) = delete;
      ELFFile& operator=(const ELFFile& other) = delete;

      /**
       * @brief Parses the link map of the given process and append the loaded symbols to this
       * object This function does not check the given process is associated  with this ELFFile
       * object. If this is not the case, this is considered undefined behaviour.
       * @param process
       */
      bool parseDynamicSymbols(Process& process);

      std::unique_ptr<DebugInfo> yieldDebugInfo() {
        return std::move(debug_info);
      }

      DebugInfo* getDebugInfo() {
        return debug_info.get();
      }

    private:
      /**
       * @brief Builds a new ELFFile object and parses static symbols from the file
       * This functions does not parse dwarf nor dynamic symbols.
       * @param elf_path
       */
      explicit ELFFile(const fs::path& elf_path);

      // Load the entire file into memory
      void loadIntoMemory(const fs::path& elf_path);

      // Parse a string table from memory
      std::string parseStringTable(const Elf64_Shdr& str_header);

      // Fill the sections vector
      bool parseSections();

      static std::string parseLiveString(Process& process, Elf64_Addr str_addr);

      std::vector<std::pair<Elf64_Addr, std::string>> parseLinkMap(Process& process,
                                                                   Elf64_Addr link_map_addr);

      // Fill the symbols table
      void parseSymbols();

      // Locate the link map of the given process
      Elf64_Addr locateLinkMap(const Process& process);

      Elf64_Ehdr* header;
      std::unique_ptr<char[]> data;
      size_t data_size = 0;

      std::filesystem::path elf_path;

      std::unique_ptr<DebugInfo> debug_info;
      std::vector<Elf64_Shdr> sections;

      bool badbit;
    };

    ELFFile::ELFFile(const fs::path& elf_path)
        : debug_info(std::make_unique<DebugInfo>()), badbit(false) {

      loadIntoMemory(elf_path);

      // We should not wrap the memory return by libelf using unique_ptr since
      // Malloc and New are not supposed to be compatible and free/delete may throw
      Elf* elf = elf_memory(data.get(), std::filesystem::file_size(elf_path));
      if (not elf) {
        badbit = true;
        return;
      }

      header = elf64_getehdr(elf);
      if (not header) {
        badbit = true;
        return;
      }

      parseSections();
      if (sections.empty()) {
        badbit = true;
        return;
      }

      // Parse the local symbols of the file (Only functions)
      parseSymbols();
      if (not debug_info->getSymbolTable()) {
        badbit = true;
        return;
      }

      readDwarfDebugInfo(elf, *debug_info.get());
      elf_end(elf);
    }

    ELFFile::~ELFFile() {
      // if (header) free(header);
    }

    void ELFFile::loadIntoMemory(const fs::path& elf_path) {
      std::ifstream file(elf_path, std::ios::binary);
      if (!file.is_open()) { throw std::runtime_error("Failed to open file " + elf_path.string()); }

      this->elf_path = elf_path;
      file.seekg(0, std::ios::end);
      auto size = file.tellg();
      file.seekg(0, std::ios::beg);

      data = std::make_unique<char[]>(size);
      file.read(data.get(), size);
      data_size = size;
    }

    // String in the elf file are stored in the following format:
    // '\0' + string + '\0' + string + '\0' + ...
    // We load the entire string table into memory without modifying it format
    // Strings can then be created from the string table by using an offset
    // Returns an empty string on failure
    std::string ELFFile::parseStringTable(const Elf64_Shdr& str_header) {
      std::string string_table;
      string_table.resize(str_header.sh_size);

      if (not data) return "";

      if (str_header.sh_offset > data_size) return "";
      char* dptr = data.get() + str_header.sh_offset;

      std::memcpy(string_table.data(), dptr, str_header.sh_size);

      if (string_table[0] != '\0') { return ""; }
      return string_table;
    }

    // Build a vector of all the sections in the elf file
    bool ELFFile::parseSections() {
      sections.resize(header->e_shnum);

      if (header->e_shoff > data_size) return false;
      char* dptr = data.get() + header->e_shoff;

      // Parse the all sections in one go
      auto* sec_ptr = sections.data();
      std::memcpy(sec_ptr, dptr, header->e_shnum * sizeof(Elf64_Shdr));
      return true;
    }

    void ELFFile::parseSymbols() {
      // This class contains a symbol table and its associated string table
      // Multiple SymbolTable can be linked together
      std::unique_ptr<SymbolTable> symbols = nullptr;

      // Parse every sections and only keep the one that contains the symbol table
      for (auto& sec : sections) {
        if (sec.sh_type != SHT_SYMTAB) continue;

        auto sym_str_table = parseStringTable(sections[sec.sh_link]);
        if (sym_str_table.empty()) continue;

        // Jump to the beginning of the symbol table
        char* dptr = data.get() + sec.sh_offset;
        size_t n_sym = sec.sh_size / sizeof(Elf64_Sym);
        auto buff = std::make_unique<SymbolTable>(n_sym, elf_path);


        for (size_t i = 0; i < n_sym; i++) {
          Elf64_Sym sym;

          char* sym_ptr = dptr + i * sizeof(Elf64_Sym);
          std::memcpy(&sym, sym_ptr, sizeof(Elf64_Sym));

          if (ELF64_ST_TYPE(sym.st_info) != STT_FUNC or sym.st_shndx > sections.size() or
              sym.st_shndx == SHN_UNDEF) {
            continue;
          }

          std::string name(sym_str_table.data() + sym.st_name);
          buff->emplace_back(sym.st_value, name, "");
        }
        if (not symbols) {
          symbols = std::move(buff);
        } else
          symbols->join(std::move(buff));
      }

      // Since we only parse functions, we may have allocated too much space
      // We shrink the table to the actual size
      // This also removes empty tables in the linked list
      symbols->shrinkToFit();
      debug_info->setSymbolTable(std::move(symbols));
    }

    // Locate the address of the link map
    // Return 0 on failure
    Elf64_Addr ELFFile::locateLinkMap(const Process& process) {
      // Locate the .dynamic section
      auto dynsec_it = std::find_if(sections.begin(), sections.end(),
                                    [](const Elf64_Shdr& s) { return s.sh_type == SHT_DYNAMIC; });

      if (dynsec_it == sections.end()) return 0;

      auto& dynsec = *dynsec_it;
      size_t ndyn = dynsec.sh_size / sizeof(Elf64_Dyn);
      std::cout << "Found " << ndyn << " dynamic entries" << std::endl;

      // Parse the section entries until we find the DT_DEBUG
      for (size_t i = 0; i < ndyn; i++) {

        Elf64_Addr curr_addr = dynsec.sh_addr + i * sizeof(Elf64_Dyn);
        long d_tag = ptrace(PTRACE_PEEKDATA, process.getPid(), curr_addr, nullptr);

        if (d_tag == -1) break;

        if (d_tag == DT_DEBUG) {
          // The debug struct addr is the second member of the struct

          auto r_debug_addr = (Elf64_Addr) ptrace(PTRACE_PEEKDATA, process.getPid(),
                                                  curr_addr + sizeof(Elf64_Sxword), 0);
          if (r_debug_addr == 0 or errno) {
            std::cerr << "Error while reading DT_DEBUG" << std::endl;
            perror("ptrace");
            return 0;
          }

          // From the debug struct we can fetch the link_map's head address
          auto link_map_addr =
                  (Elf64_Addr) ptrace(PTRACE_PEEKDATA, process.getPid(),
                                      r_debug_addr + offsetof(struct r_debug, r_map), 0);

          return link_map_addr;
        }
      }
      return 0;
    }

    std::string ELFFile::parseLiveString(Process& process, Elf64_Addr str_addr) {

      std::string str;
      str.resize(256);

      memset(str.data(), 0, 256);

      Elf64_Addr curr_addr = ptrace(PTRACE_PEEKDATA, process.getPid(), str_addr, 0);
      if (curr_addr <= 1) return "";

      // ptrace allows us to read sizeof(long) bytes at a time
      // So we cannot read more than len(str) / sizeof(long) bytes
      // Cast to long* for reading
      long* dptr = reinterpret_cast<long*>(str.data());

      // Ensure we have enough space to read an entire long
      // And that the length of the string is exactly equal to the number of byte read
      // If the second condition is false, it means we read '\0'
      for (size_t i = 0; str.size() - i * sizeof(long) >= 0 and str.size() >= i * sizeof(long);
           i++) {
        dptr[i] = ptrace(PTRACE_PEEKDATA, process.getPid(), curr_addr + i * sizeof(long), 0);
        if (str[i] <= 0) break;
      }
      str.resize(str.find('\0'));
      str.shrink_to_fit();
      return str;
    }

    std::vector<std::pair<Elf64_Addr, std::string>>
    ELFFile::parseLinkMap(Process& process, Elf64_Addr link_map_addr) {
      // The link_map is stored as a linked list
      // But we don't want to handle malloc, so we store them in a vector
      std::vector<std::pair<Elf64_Addr, std::string>> res;
      pid_t pid = process.getPid();
      bool done = false;

      link_map_addr = (Elf64_Addr) ptrace(PTRACE_PEEKDATA, pid,
                                          link_map_addr + offsetof(struct link_map, l_next), 0);

      while (not done) {
        // Writing in place
        res.emplace_back();
        auto& curr = res.back();
        /* base address */
        curr.first = (Elf64_Addr) ptrace(PTRACE_PEEKDATA, pid,
                                         link_map_addr + offsetof(struct link_map, l_addr), 0);
        if (curr.first <= 0) {
          res.pop_back();
          break;
        }

        curr.second = parseLiveString(process, link_map_addr + offsetof(struct link_map, l_name));

        // Address of the next item
        link_map_addr = (Elf64_Addr) ptrace(PTRACE_PEEKDATA, pid,
                                            link_map_addr + offsetof(struct link_map, l_next), 0);
        if (link_map_addr <= 0 or errno) { done = true; }
      }
      return res;
    }

    bool ELFFile::parseDynamicSymbols(Process& process) {

      auto dynsec = std::find_if(sections.begin(), sections.end(),
                                 [](const Elf64_Shdr& sec) { return sec.sh_type == SHT_DYNSYM; });
      if (dynsec == sections.end()) return false;

      auto link_map_addr = locateLinkMap(process);
      if (not link_map_addr) return false;


      std::unique_ptr<SymbolTable> res = nullptr;
      auto link_map = parseLinkMap(process, link_map_addr);

      for (const auto& lm : link_map) {
        try {
          ELFFile elf(lm.second);
          auto deb_info = elf.yieldDebugInfo();
          auto new_symbols = deb_info->yieldSymbolTable();
          new_symbols->relocate(lm.first);

          if (not res) res = std::move(new_symbols);
          else
            res->join(std::move(new_symbols));
          debug_info->appendSharedLibraries(lm.second);
        } catch (std::runtime_error& e) { std::cout << e.what() << std::endl; }
      }

      auto* old_symtab = debug_info->getSymbolTable();
      if (not old_symtab) {
        debug_info->setSymbolTable(std::move(res));
      } else {
        old_symtab->join(std::move(res));
      }

      return true;
    }
  }// namespace

  // Parse the ELF file and return an object containing all debug information required for the
  // debugger. For this, we use both libelf and libdwarf.
  //
  // Libelf is use to parse the file after it
  // has been loaded in memory (Otherwise libdwarf requires a file descriptor)
  std::unique_ptr<const DebugInfo> readDebugInfo(const std::filesystem::path& path,
                                                 Process& process) {

    auto elf_file = ELFFile::make(path);
    if (not elf_file) return nullptr;
    if (not process.isAttached() and not process.attach()) return nullptr;
    elf_file->parseDynamicSymbols(process);
    return elf_file->yieldDebugInfo();
  }

}// namespace ldb
#pragma once
#include "ELFFile.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>

namespace ldb {

  class ELFReader {
  public:
    static std::unique_ptr<ELFFile> read(const std::filesystem::path& path, pid_t pid);

  private:
    static std::optional<Elf64_Ehdr> parseHeader(std::istream& stream);

    static std::optional<std::string> parseStringTable(std::istream& istream, Elf64_Ehdr header);

    static std::vector<Section> parseSections(std::istream& stream, const Elf64_Ehdr& header,
                                              const std::string& String_table);

    static std::shared_ptr<SymbolList> parseSymbols(std::istream& stream, const Elf64_Ehdr& header,
                                                    const std::string& String_table);
  };

}// namespace ldb

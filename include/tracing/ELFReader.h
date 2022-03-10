#pragma once
#include "DebugInfo.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>

namespace ldb {

  class ELFReader {
  public:
    static std::unique_ptr<DebugInfo> read(const std::filesystem::path& path, pid_t pid);

  private:
    static std::optional<Elf64_Ehdr> parseHeader(std::istream& stream);

    static std::optional<std::string> parseStringTable(std::istream& istream, size_t offset);

    static std::vector<Section> parseSections(std::istream& stream, const Elf64_Ehdr& header,
                                              const std::string& String_table);

    static std::unique_ptr<SymbolTable> parseSymbols(std::istream& stream,
                                                    const std::vector<Section>& sections);
  };

}// namespace ldb

#pragma once
#include "Section.h"
#include "SymbolList.h"
#include <filesystem>
#include <memory>
#include <vector>

namespace ldb {

  /**
   * @brief Objects that contains debug informations containted in an ELF file.
   * This includes symbols, sections, frame infos, and dynamic entries
   *
   * Multiple elf files should be merged together to obtain debug information of a program.
   */
  class ELFFile {
    friend class ELFReader;

  public:
    ELFFile(const ELFFile& other) = delete;
    ELFFile& operator=(const ELFFile& other) = delete;


    /**
     * @return returns a vector of all the sections contained in this file
     */
    std::vector<Section>& getSections() {
      return sections;
    }

    /**
     * @return returns a vector of all the sections contained in this file
     */
    const std::vector<Section>& getSections() const {
      return sections;
    }

    /**
     * @brief Returns the section with the given name
     * @param sname The name of the section to find
     * @return A pointer to the section if found, nullptr otherwise
     */
    Section* getSection(const std::string& sname);

    /**
     * @brief Returns the section with the given name
     * @param sname The name of the section to find
     * @return A pointer to the section if found, nullptr otherwise
     */
    const Section* getSection(const std::string& sname) const;

    /**
     * @return Returns the number of sections in this file
     */
    size_t getSectionCount() const;


    std::shared_ptr<SymbolList> getSymbols() {
      return symbols;
    }

    std::shared_ptr<const SymbolList>& getSymbols() const;


  private:
    ELFFile(std::filesystem::path elf_path);

    std::filesystem::path file_path;
    std::vector<Section> sections;
    std::shared_ptr<SymbolList> symbols;
  };

}// namespace ldb

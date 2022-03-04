#pragma once
#include <elf.h>
#include <iostream>

namespace ldb {

  class ELFFile;

  enum class SectionType {
    kUnknown = 0,
    kProgBits = 1,
    kSymTab = 2,
    kStrTab = 3,
    kRela = 4,
    kHash = 5,
    kDyn = 6,
    kNote = 7,
    kNoBits = 8,
    kRel = 9,
    kShLib = 10,
    kDynSym = 11,
    kInitArray = 14,
    kFiniArray = 15,
    kPreInitArray = 16,
    kGroup = 17,
    kSymTabShndx = 18,
    kNum = 19
  };

  class Section {
  public:
    Section(SectionType type, std::string name, size_t hdr_offset, size_t section_offset,
            size_t size, Elf64_Addr vaddr);

    SectionType getType() const {
      return type;
    }

    std::string getName() const {
      return name;
    }

    size_t getHdrOffset() const {
      return hdr_offset;
    }

    size_t getSectionOffset() const {
      return section_offset;
    }

    size_t getSize() const {
      return section_size;
    }

    size_t getVirtualAddress() const {
      return virtual_address;
    }

  private:
    SectionType type;

    std::string name;
    size_t section_offset;
    size_t section_size;

    size_t hdr_offset;
    Elf64_Addr virtual_address;
  };
}// namespace ldb
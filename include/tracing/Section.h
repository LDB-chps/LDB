#pragma once
#include <elf.h>
#include <iostream>

namespace ldb {
  enum class SectionType { kUnknown, kElf, kDwarf };

  class ELFFile;

  class Section {
  public:
    Section(SectionType type, std::string name, size_t file_offset, Elf64_Addr vaddr, size_t size);

    SectionType getType() const {
      return type;
    }

    std::string getName() const {
      return name;
    }

    size_t getFileOffset() const {
      return file_offset;
    }

    size_t getSize() const {
      return size;
    }

    size_t getVirtualAddress() const {
      return virtual_address;
    }

  private:
    std::string name;
    size_t file_offset;
    size_t size;
    Elf64_Addr virtual_address;
    SectionType type;
  };
}// namespace ldb
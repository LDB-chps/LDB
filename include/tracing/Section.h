#pragma once
#include <iostream>

namespace ldb {
  enum class SectionType { kUnknown, kElf, kDwarf };

  class Section {
  public:
    Section(SectionType type, std::string name, size_t file_offset);

    SectionType getType() const {
      return type;
    }

    std::string getName() const {
      return name;
    }

    size_t getFileOffset() const {
      return file_offset;
    }

  private:
    ELFFile* parent;
    std::string name;
    size_t file_offset;
    SectionType type;
  };
}// namespace ldb
#include "Section.h"

#include <utility>

namespace ldb {
  Section::Section(ELFFile* parent, SectionType type, std::string name, size_t file_offset)
      : parent(parent), type(type), name(std::move(name)), file_offset(file_offset) {
  }

}
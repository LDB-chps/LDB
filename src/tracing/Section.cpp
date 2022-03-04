#include "Section.h"

#include <utility>

namespace ldb {
  Section::Section(SectionType type, std::string name, size_t file_offset, Elf64_Addr vaddr,
                   size_t size)
      : type(type), name(std::move(name)), file_offset(file_offset), virtual_address(vaddr),
        size(size) {}

}// namespace ldb
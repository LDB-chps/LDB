#include "Section.h"

#include <utility>

namespace ldb {
  Section::Section(SectionType type, std::string name, size_t hdr_offset, size_t section_offset,
                   size_t size, Elf64_Addr vaddr)
      : type(type), name(std::move(name)), hdr_offset(hdr_offset), section_offset(section_offset),
        section_size(size), virtual_address(vaddr) {}

}// namespace ldb
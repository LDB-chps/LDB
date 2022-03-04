#pragma once

#include <iostream>
#include <memory>
#include "Section.h"
#include "FrameTable.h"
#include "DebugInfo.h"

namespace ldb {

class EHFrameReader {
public:
  EHFrameReader();
  ~EHFrameReader();

  EHFrameReader(EHFrameReader&& other) = default;
  EHFrameReader& operator=(EHFrameReader&& other) = default;

  EHFrameReader(const EHFrameReader&) = delete;
  EHFrameReader&operator=(const EHFrameReader&) = delete;

  std::unique_ptr<FrameTable> read(DebugInfo& elf_file, Section& eh_frame_section);

private:


};

}
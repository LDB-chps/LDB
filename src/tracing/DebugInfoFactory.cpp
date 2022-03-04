#include "DebugInfoFactory.h"
#include "ELFReader.h"


namespace ldb {

  std::unique_ptr<const DebugInfo>
  DebugInfoFactory::load(std::filesystem::path const& executable_path, pid_t pid) {
    std::unique_ptr<DebugInfo> res(new DebugInfo());
    res->executable_path = executable_path;

    // Recursively parse the executable file
    if (not parseElf(executable_path, *res, pid, 0)) return nullptr;

    return res;
  }

  bool DebugInfoFactory::parseElf(const std::filesystem::path& elf_path, DebugInfo& debug_info,
                                  pid_t pid, size_t relocation_base) {
    auto elf_file = ELFReader::read(elf_path, pid);

    if (not elf_file) return false;

    // Append the symbols to the SymbolsTable
    auto& symbols = elf_file.getSymbols();
    if (relocation_base != 0) symbols.relocate(relocation_base);
    debug_info.symbols_table.join(symbols);


    // Append the frame infos to the frame table
    auto& frame_infos = elf_file.getFrameInfos();
    if (relocation_base != 0) frame_infos.relocate(relocation_base);
    debug_info.frame_infos.join(frame_infos);

    for (auto& entry : elf_file.getDynamicEntries()) {
      parseElf(entry.getPath(), debug_info, entry.getAddress());
    }

    return true;
  }
}// namespace ldb
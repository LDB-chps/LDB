#pragma once

#include "DebugInfo.h"
#include <iostream>
#include <libdwarf/dwarf.h>
#include <libdwarf/libdwarf.h>
#include <map>
#include <sstream>
#include <string>
#include <vector>


// delete it
#include <fcntl.h>


namespace ldb {

  void readDwarfDebugInfo(Elf* elf, DebugInfo& db);

}// namespace ldb
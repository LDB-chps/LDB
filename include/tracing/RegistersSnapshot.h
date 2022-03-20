#pragma once
#include "Process.h"
#include <iostream>
#include <vector>

namespace ldb {

  /**
   * @brief Represents a snapshot of one registers of a process.
   * Associates a name with a value
   */
  class RegisterValue {
  public:
    RegisterValue(const std::string& name, long value) : name(name), value(value) {}

    const std::string& getName() const {
      return name;
    }

    std::string getValueAsString() {
      char buffer[64] = {'0', 'x'};
      sprintf(buffer + 2, "%lx", value);
      return {buffer};
    }

    long getValue() const {
      return value;
    }

  private:
    const std::string name;
    long value;
  };

  /**
   * @brief Represents a snapshot of all the registers of the process
   * Currently only x86_64 is supported, and dies not handle FP registers
   */
  class RegistersSnapshot {
  public:
    explicit RegistersSnapshot(Process& process);

    using iterator = std::vector<RegisterValue>::iterator;
    using const_iterator = std::vector<RegisterValue>::const_iterator;

    iterator begin() {
      return registers.begin();
    }

    iterator end() {
      return registers.end();
    }

    const_iterator begin() const {
      return registers.begin();
    }

    const_iterator end() const {
      return registers.end();
    }

  private:
    std::vector<RegisterValue> registers;
  };

}// namespace ldb

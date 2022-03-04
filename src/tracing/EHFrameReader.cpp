#include "EHFrameReader.h"


namespace ldb {

  namespace {

    enum class TokenType { kUndefined, kCIE, kFDE, kTerminator };


    struct Token {

      size_t length = 0;
      size_t file_offset = 0;
      TokenType type = TokenType::kUndefined;
    };

    Token readToken(std::istream stream) {
      Token res;

      res.file_offset = stream.tellg();

      int size = 0;
      stream.read(reinterpret_cast<char*>(&size), sizeof(size));

      if (size == 0) {
        res.type = TokenType::kTerminator;
        return res;
      } else if (size == 0xFFFFFFFF) {
        size_t extended_size = 0;
        stream.read(reinterpret_cast<char*>(&extended_size), sizeof(extended_size));
        res.length = extended_size + 12;
      } else {
        res.length = size + 4;
      }

      int id = 0;
      stream.read(reinterpret_cast<char*>(&id), sizeof(id));

      if (id == 0) {
        res.type = TokenType::kCIE;
      } else if (id == 1) {
        res.type = TokenType::kFDE;
      } else {
        res.type = TokenType::kUndefined;
        return res;
      }
    }
  }// namespace

  std::unique_ptr<FrameTable> EHFrameReader::read(DebugInfo& elf_file, Section& eh_frame_section) {
    if (eh_frame_section.getType() != Section::kElf or eh_frame_section.getName() != ".eh_frame") {
      return nullptr;
    }

    eh_frame = std::make_unique<FrameTable>();

    bool done = false;
    bool error = false;

    auto stream = elf_file.getStream();
    size_t curr_offset = eh_frame_section.getOffset();

    while (not done and not error) {
      Token token = readToken(stream);

      if (token.type == TokenType::kFDE) {
        readFDE(token, stream);
      } else if (token.type == TokenType::kCIE) {

        readCIE(token, stream);
      } else if (token.type == TokenType::kTerminator) {
        std::cout << "Found the Terminator" << std::endl;
        done = true;
        continue;
      } else if (token.type == TokenType::kUndefined) {
        std::cout << "Found an undefined token" << std::endl;
        error = true;
        continue;
      }

      // Ensure we are at the next token
      stream.lseekg(curr_offset, std::ios::beg);
      stream.lseekg(token.length, std::ios::cur);
      curr_offset = stream.tellg();
    }

    if (not done or error) return nullptr;
    return eh_frame;
  }

  bool EHFrameReader::readCIE(Token token, std::istream stream) {
    auto buffer = std::make_unique<char>(token.length);
    stream.read(buffer.get(), token.length);

    std::cout << "Found a CIE !" << std::endl;
  }

  bool EHFrameReader::readFDE(Token token, std::istream stream) {
    auto buffer = std::make_unique<char>(token.length);
    stream.read(buffer.get(), token.length);

    std::cout << "Found a FDE !" << std::endl;
  }
} // namespace ldb
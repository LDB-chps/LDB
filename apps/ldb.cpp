#include "ldbapp.h"
#include <iostream>


#include <elf.h>
#include <fcntl.h>
#include <list>
#include <string.h>
#include <string>
#include <unistd.h>


typedef struct _t_Symbols {
  struct _t_Symbols* next;
  char* name;
  Elf64_Sym* symbols;
  int nsyms;
  char* strings;
  int strslen, noffsets;
  Elf64_Addr baseAddr;
  Elf64_Dyn* dynamic;
  int ndyns;
} Symbols;


void foo() {
  std::string exe("/home/johnkyky/Documents/readElf/pid");
  std::cout << exe << std::endl;

  int fd = 0;
  if ((fd = open(exe.c_str(), O_RDONLY)) < 0) { throw std::runtime_error("open failed"); }
  std::cout << "file descriptor : " << fd << std::endl;

  /*-------------------------------------------------------------------------------------------------*/

  Elf64_Ehdr header;
  if (read(fd, &header, sizeof(header)) < (int) sizeof(header)) {
    throw std::runtime_error("read failed");
  }

  std::cout << "header.e_type: " << header.e_type << std::endl;

  /*-------------------------------------------------------------------------------------------------*/

  Elf64_Shdr section_header;
  memset(&section_header, 0, sizeof(section_header));
  Symbols* syms = new Symbols;

  int cursor_pos = header.e_shoff;
  lseek(fd, cursor_pos, SEEK_SET);

  std::cout << header.e_shnum << " section header find" << std::endl;

  printf("section header size of one %u\n", header.e_shentsize);
  printf("section header number %u\n", header.e_shnum);


  for (Elf64_Half i = 0; i < header.e_shnum; i++) {
    read(fd, &section_header, sizeof(section_header));


    /*if (section_header.sh_type == SHT_SYMTAB) {
      std::cout << "c est la bonne" << std::endl;

      syms->nsyms = section_header.sh_size / sizeof(Elf64_Sym);
      std::cout << "il y a " << syms->nsyms << " symboles" << std::endl;

      syms->symbols = (Elf64_Sym*) malloc(section_header.sh_size);
      lseek(fd, section_header.sh_offset, SEEK_SET);
      read(fd, syms->symbols, section_header.sh_size);

      std::cout << syms->symbols[0].st_size << std::endl;
    }*/

    /*switch (section_header.sh_type) {
      case SHT_SYMTAB:
        syms->nsyms = section_header.sh_size / sizeof(Elf64_Sym);

        if (!(syms->symbols = (Elf64_Sym*) malloc(section_header.sh_size)))
          throw std::runtime_error("Could not allocate symbol table.");

        if ((Elf64_Off) lseek(fd, section_header.sh_offset, SEEK_SET) != section_header.sh_offset ||
            (uint64_t) read(fd, syms->symbols, section_header.sh_size) != section_header.sh_size)
          throw std::runtime_error("Could not read symbol table.");

        i = header.e_shoff + section_header.sh_link * header.e_shentsize;
        if (lseek(fd, i, SEEK_SET) != i) throw std::runtime_error("Could not seek and find.");

        if (read(fd, &section_header, header.e_shentsize) != header.e_shentsize)
          throw std::runtime_error("Could not read string table section header.");

        if (!(syms->strings = (char*) malloc(section_header.sh_size)))
          throw std::runtime_error("Could not allocate string table.");

        if ((Elf64_Off) lseek(fd, section_header.sh_offset, SEEK_SET) != section_header.sh_offset ||
            (uint64_t) read(fd, syms->strings, section_header.sh_size) != section_header.sh_size)
          throw std::runtime_error("Could not read string table.");
        break;
      case SHT_DYNSYM:
        break;
      default:
        break;
    }*/

    cursor_pos += header.e_shentsize;
    lseek(fd, cursor_pos, SEEK_SET);
  }

  close(fd);
}


int main(int argc, char** argv) {
  std::cout << "Hello, World!" << std::endl;
  // ldb::LDBApp app;
  // app.run(argc, argv);

  foo();

  return 0;
}
#ifndef MOS_INCLUDE_ELF_H
#define MOS_INCLUDE_ELF_H

#include "types.h"

#define ELF_MAGIC 0x464C457FU

//std
struct ELF
{
    uint32_t e_magic;
    uint8_t e_elf[12];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
   uint16_t e_shentsize;
   uint16_t e_shnum;
   uint16_t e_shstrndx;
};
//std
struct Proghdr
{
    uint32_t p_type;
    uint32_t p_offset;
    uint32_t p_va;
    uint32_t p_pa;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_flags;
    uint32_t aligns;
};
//std
struct Secthdr
{
   uint32_t sh_name;
   uint32_t sh_type;
   uint32_t sh_flags;
   uint32_t sh_addr;
   uint32_t sh_offset;
   uint32_t sh_size;
   uint32_t sh_link;
   uint32_t sh_info;
   uint32_t sh_addralign;
   uint32_t sh_entsize;
};

#define ELF_PROG_LOAD 1

#define ELF_PROG_FLAG_EXECUTE 1

#define ELF_PROG_FLAG_WRITE 2

#define ELF_PROG_FLAG_READ 4

#define ELF_SHT_NULL 0

#define ELF_SHT_PROGBITS 1

#define ELF_SHT_SYMTAB 2

#define ELF_SHT_STRRAB 3

#define ELF_SHN_UNDEF 0

#endif

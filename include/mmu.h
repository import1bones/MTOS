#ifndef MOS_INCLUDE_MANAGER_MEMORY_UNIT_H
#define MOS_INCLUDE_MANAGER_MEMORY_UNIT_H
//manager memory unit
// & 0X3FF -> & 00000000 00000000 00001111 11111111(bin) 32 bits ,
//which & with 0X3FF,which's high 20 bits set to zero and low 12 bits hold.
#define PAGE_NUMBER(linear_address)	(((uintptr_t) (linear_address)) >> PTXSHIFT)

#define PAGE_DIRECTORY_INDEX(linear_address) ((((uintptr_t) (linear_address)) >> PDXSHIFT) & 0X3FF)

#define PAGE_TABLE_INDEX(linear_address) ((((uintptr_t) (linear_address)) >> PTXSHIFT) & 0X3FF)

#define PAGE_OFFSET(linear_address) (((uintptr_t) (linear_address)) & 0XFFF)

#define PAGE_ADDRESS(d, t, o) ((void*) ((d) << PDXSHIFT | (t) <<PTXSHIFT | (O)))

#define NUMBER_PAGE_DIRECTORY_ENTRIES 1024
#define NUMBER_PAGE_DIRECTORY_ENTRIES 1024
#define PAGE_SIZE 4096
//1ki * 1ki * 4ki = 4GB

#define PAGE_TABLE_SIZE (PAGE_SIZE * NUMBER_PAGE_DIRECTORY_ENTRIES
#define PGSHIFT 12
#define PTSHIFT 22
#define PTXSHIFT 12
#define PDXSHIFT 22
//PAGE table Directory sign
#define PAGE_DIRECTORY_ENTRY_P 0x001 //present
#define PAGE_DIRECTORY_ENTRY_W 0x002 //write/read
#define PAGE_DIRECTORY_ENTRY_U 0x004 //user/supervisor
#define PAGE_DIRECTORY_ENTRY_PWT 0x008
#define PAGE_DIRECTORY_ENTRY_PCD 0x010
#define PAGE_DIRECTORY_ENTRY_A 0x020
#define PAGE_DIRECTORY_ENTRY_0 0x040
#define PAGE_DIRECTORY_ENTRY_PS 0x080
#define PAGE_DIRECTORY_ENTRY_G 0x100
//question : why 0xE00,bit[11-9]
//ans: set bit[11-9]={1,1,1}
#define PAGE_DIRECTORY_ENTRY_AVAIL 0xE00

#define PAGE_DIRECTORY_ENTRY_ADDRESS(pte) ((physaddr_t) (pte) & ~0xFFF)

//PAGE table entry sign
#define PAGE_TABLE_ENTRY_P 0x001
#define PAGE_TABLE_ENTRY_W 0x002
#define PAGE_TABLE_ENTRY_U 0x004
#define PAGE_TABLE_ENTRY_PWT 0x008
#define PAGE_TABLE_ENTRY_PCD 0x010
#define PAGE_TABLE_ENTRY_A 0x020
#define PAGE_TABLE_ENTRY_D 0x040
#define PAGE_TABLE_ENTRY_PAT 0x080
#define PAGE_TABLE_ENTRY_G 0x100
#define PAGE_TABLE_ENTRY_AVAIL 0xE00

//control register
#define CONTROL_REGISTER_0_PE 0x00000001
#define CONTROL_REGISTER_0_MP 0x00000002
#define CONTROL_REGISTER_0_EM 0x00000004
#define CONTROL_REGISTER_0_TS 0x00000008
#define CONTROL_REGISTER_0_ET 0x00000010
#define CONTROL_REGISTER_0_NE 0x00000020
#define CONTROL_REGISTER_0_WP 0x00010000
#define CONTROL_REGISTER_0_AM 0x00040000
#define CONTROL_REGISTER_0_NW 0x20000000
#define CONTROL_REGISTER_0_CD 0x40000000
#define CONTROL_REGISTER_0_PG 0x80000000

#define CONTROL_REGISTER_3_PWT 0x00000008
#define CONTROL_REGISTER_3_PCD 0x00000010

#define CONTROL_REGISTER_4_VME 0x00000001
#define CONTROL_PEGISTER_4_PVI 0x00000002
#define CONTROL_REGISTER_4_TSD 0x00000004
#define CONTROL_REGISTER_4_ED 0x00000008
#define CONTROL_REGISTER_4_PSE 0x00000010
#define CONTROL_REGISTER_4_PAE 0x00000020
#define CONTROL_REGISTER_4_MCE 0x00000040
#define CONTROL_REGISTER_4_PGE 0x00000080
#define CONTROL_REGISTER_4_PCE 0x00000100
#define CONTROL_REGISTER_4_OSFXSR 0x00000200
#define CONTROL_REGISTER_4_OSXMMEXCPT 0x00000400
#define CONTROL_REGISTER_4_VMXE 0x00004000

//system flags in EFLAGS Register
#define EFLAGS_RIGISTER_CF 0x00000001
#define EFLAGS_RIGISTER_PF 0x00000004
#define EFLAGS_RIGISTER_AF 0x00000010
#define EFLAGS_RIGISTER_ZF 0x00000040
#define EFLAGS_REGISTER_SF 0x00000080
#define EFLAGS_REGISTER_TF 0x00000100
#define ELFAGS_REGISTER_IF 0x00000200
#define ELFAGS_REGISTER_DF 0x00000400
#define ELFAGS_REGISTER_OF 0x00000800
#define ELFAGS_REGISTER_IOPL_0 0x00000000
#define ELFAGS_REGISTER_IOPL_1 0x00001000
#define ELFAGS_REGISTER_IOPL_2 0x00002000
#define ELFAGS_REGISTER_IOPL_3 0x00003000
#define ELFAGS_REGSITER_NT 0x00004000
#define ELFAGS_REGISTER_RF 0x00010000
#define ELFAGS_REGISTER_VM 0x00020000
#define ELFAGS_REGISTER_AC 0x00040000
#define ELFAGS_REGISTER_VIF 0x00080000
#define ELFAGS_REGISTER_VIP 0x00100000
#define ELFAGS_REGISTER_ID 0x00200000

#ifdef __ASSEMBLER__

#define SEGMENT_NULL    \
    .word 0, 0;  \
    .byte 0, 0, 0, 0;  
//question for segment
#define SEGMENT(type, base, limit)  \
    .word (((limit) >> 12) & 0xFFFF),((base) & 0xFFFF); \
    .byte (((base) >> 16) & 0xFF),(0x90 | (type)),(0xC0 | (((limit) >> 28) & 0xF)),(((base) >> 24) & 0xFF);
#else
#include<include/types.h>
struct segment_descriptor
{
    unsigned segment_limit_15_0 : 16;
    unsigned base_limit_15_0 : 16;
    unsigned base_limit_23_16 : 8;
    unsigned segment_type : 4;
    unsigned segment_s : 1;
    //descriptor privilege level
    unsigned segment_dpl : 2;
    unsigned segment_p : 1;
    unsigned segment_limit_19_16 : 4;
    unsigned segment_avail : 1;
    unsigned segment_reserve : 1;
    unsigned segment_db : 1;
    unsigned segment_g : 1;
    unsigned base_31_24 : 8;
};

#define SEGMENT_NULL {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#define SEGMENT_FAULT {0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0};

#define SEGMENT(type, base, limit, dpl) \
{   \
    ((limit) >> 12) & 0xFFFF,   \
    (base) & 0xFFFF,    \
    ((base) >> 16) & 0xFF,  \  
    type,   \
    1,  \
    dpl,    \
    1,  \
    (unsigned) (limit) >> 28,   \
    0,  \
    0,  \
    1,  \
    1,  \
    (unsigned) (base) >> 24 \
}

#define SEGMENT_16(type, base, limit, dpl) (struct segment_descriptor)  \
{   \
    (limit) & 0xFFFF,   \
    (base) & 0xFFFF,    \
    ((base) >> 16) & 0xff,  \
    type,   \
    1,  \
    dpl,    \
    1,  \
    (unsigned) (limit) >> 16,   \
    0,  \
    0,  \
    1,  \
    0,  \
    (unsigned) (base) >> 24 \
}
#endif
#define SEGMENT_TYPE_X 0x8
#define SEGMENT_TYPE_E 0x4
#define SEGMENT_TYPE_C 0x4
#define SEGMENT_TYPE_W 0x2
#define SEGMENT_TYPE_R 0x2
#define SEGMENT_TYPE_A 0x1

#endif

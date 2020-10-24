#include<include/elf.h>
#include<include/x86.h>

#define SECTION_SIZE 512
#define ELFHDR ((struct ELF*) 0x10000)

void read_section(void*, uint32_t);
void read_segment(uint32_t, uint32_t, uint32_t);

void bootmain(void)
{
    struct Proghdr *ph, eph;
    read_segment((uint32_t) ELFHDR, SECTION_SIZE * 8, 0);
    if(ELFHDR->e_magic!=ELF_MAGIC)
    {
        goto bad;
    }
    ph = (struct Proghdr*) ((uint8_t*) ELFHDR + ELFHDR->e_phoff);
    eph=ph+ELFHDR->e_phnum;
    while(ph < eph)
    {
        read_segment(ph->p_pa, ph->p_memsz, ph->p_offset);
        ph++;
    }
    ((void (*)(void)) (ELFHDR->e_entry))();

    bad:
        out_word_from_port(0x8A00, 0x8A00);
        out_word_from_port(0x8A00, 0x8E00);
        while(1)
            ;/*do nothing*/
}
void read_segment(uint32_t pa, uint32_t count, uint32_t offset)
{
    uint32_t end_pa;
    end_pa = pa + offset;

    pa &= ~(SECTION_SIZE - 1);

    offset = (offset / SECTION_SIZE)+1;

    while(pa<end_pa)
    {
        read_section((uint8_t*) pa, offset);
        pa + = SECTION_SIZE;
        offset++;
    }
}

void wait_disk(void)
{
    while((input_byte_from_port(0x1F7) & 0xC0) != 0x40)
        ;/*do nothing*/
}

void read_section(void *dst, uint32_t offset)
{
    wait_disk();
    out_byte_from_port(0x1F2, 1);
    out_byte_from_port(0x1F3, offset);
    out_byte_from_port(0x1F4, offset >> 8);
    out_byte_from_port(0x1F5, offset >> 16);
    out_byte_from_port(0x1F6, (offset >> 24) | 0xE0);
    out_byte_from_port(0x1F7, 0x20);

    wait_disk();

    input_long_string_from_port(0x1F0, dst, SECTION_SIZE/4);
}

#ifndef MOS_INCLUDE_X86_H
#define MOS_INCLUDE_X86_H

#include<include/types.h>

static inline void breakpoint(void)
{
    asm volatile("int3");
}

static inline uint8_t input_byte_from_port(int port)
{
    uint8_t data;
    asm volatile("inb %w1,%0" : "=a" (data) : "d" (port));
    return data;
}

static inline void input_byte_string_from_part(int port, void *addr, int cnt)
{
    asm volatile("cld\n\trepne\n\tinsb"
            :   "=D" (addr), "=c" (cnt)
            :   "d" (port), "0" (addr), "1" (cnt)
            :   "memory", "cc");
}

static inline uint16_t input_word_from_port(int port)
{
    uint16_t data;
    asm volatile("inw %w1,%0" : "=a" (data) : "d" (port));
    return  data;
}

static inline void input_word_string_from_port(int port, void *addr, int cnt)
{
    asm volatile("cld\n\trepne\n\tinsw"
            :   "=D" (addr), "=c" (cnt)
            :   "d" (port), "0" (addr), "1" (cnt)
            :   "memory", "cc");
}

static inline uint32_t input_long_from_port(int port)
{
    uint32_t data;
    asm volatile("inl %w1,%0" : "=a" (data) : "d" (port));
    return data;
}

static inline void input_long_string_from_port(int port, void *addr, int cnt)
{
    asm volatile("cld\n\trepne\n\tinsl"
            :   "=D" (addr), "=c" (cnt)
            :   "d" (port), "0" (addr), "1" (cnt)
            :   "memory", "cc");
}

static inline void out_byte_from_port(int port,uint8_t data)
{
    asm volatile("outb %0,%w1" : : "a" (data), "d" (port));
}

static inline void out_word_from_port(int port, uint16_t data)
{
    asm volatile("outw %0,%w1" : : "a" (data), "d" (port));
}

static inline void out_word_string_from_port(int port, const void *addr, int cnt)
{
    asm volatile("cld\n\trepne\n\toutsl"
            : "=S" (addr), "=c" (cnt)
            : "d" (port), "0" (addr), "1" (cnt)
            : "cc");
}

static inline void out_long_from_port(int port, uint32_t data)
{
    asm volatile("outl %0,%w1" : : "a" (data), "d" (port));
}

#endif

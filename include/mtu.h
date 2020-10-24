#ifndef MOS_INCLUDE_MANAGER_TASK_UNIT_H
#define MOS_INCLUDE_MANAGER_TASK_UNIT_H
#ifdef __ASSEMBLER__
#include<include/types.h>
struct task_state_segment
{
    //only 16-bits,why using uint32_t 
    uint32_t task_state_link;
    uintptr_t task_state_esp0;
    uint16_t task_state_ss0;
    uintptr_t task_state_esp1;
    uint16_t task_state_ss1;
};

#ifndef __ASMHELPER_H__
#define __ASMHELPER_H__


//save fpu mmx xmm mxcsr registers state to memory, input one param
void save_fpu_mmx_xmm_mxcsr_state(void* addr);

void save_processor_extended_state(void* addr);

void hook_core0_isr_asm_fun(void);

#endif
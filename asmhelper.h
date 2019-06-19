#ifndef __ASMHELPER_H__
#define __ASMHELPER_H__


//save fpu mmx xmm mxcsr registers state to memory, input one param
void save_fpu_mmx_xmm_mxcsr_state_asm_fun(void* addr);

void save_processor_extended_state_asm_fun(void* addr);

void hook_core0_isr_asm_fun(void);
void hook_core1_isr_asm_fun(void);
void hook_core2_isr_asm_fun(void);
void hook_core3_isr_asm_fun(void);
void hook_core4_isr_asm_fun(void);
void hook_core5_isr_asm_fun(void);
void hook_core6_isr_asm_fun(void);
void hook_core7_isr_asm_fun(void);

#endif
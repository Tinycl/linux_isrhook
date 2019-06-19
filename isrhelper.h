#ifndef __ISRHELPER_H__
#define __ISRHELPER_H__

#include <linux/kernel.h> 
#include <linux/types.h>
#include <linux/module.h>
#include <asm/hpet.h>
#include "typehelper.h"
#include "pagehelper.h"
#include "asmhelper.h"

#define CORENUM 8
#define HPETBASE 0xfed00000

#define APICTIME_VECTOR 0xec


typedef void (*hook_isr_fun_type)(void);

extern unsigned char hook_flag ;

#pragma pack(1)

typedef struct tagIDTR
{
    uint16_t limit; //low 16 bit
    uint64_t base;  //high 64 bit
}__attribute__((packed)) IDTR, *PIDTR;

typedef struct tagGDTR
{
    uint16_t limit; //low 16 bit
    uint64_t base;  //high 64 bit
}__attribute__((packed)) GDTR, *PGDTR;

typedef struct tagIDT_GATE_DESCRIPTOR
{
    uint16_t offset0_15;
    uint16_t selector;
    uint8_t  unused;
    uint8_t  access;
    uint16_t offset16_31;
    uint32_t offset32_63;
    uint32_t reserved;
}__attribute__((packed)) IDT_GATE_DESCRIPTOR, *PIDT_GATE_DESCRIPTOR;

#pragma pack()


/* x86\include\asm\processor.h  cpuid(. . . . . ) */
void cpuid_helper(uint32_t id, uint32_t *buff);
unsigned char get_core_initial_apic_id(void);
uint64_t idt_descriptor_offset_to_va(IDT_GATE_DESCRIPTOR idt_gate);
void va_to_idt_descriptor_offset(uint64_t va, IDT_GATE_DESCRIPTOR* pidat_gate);
uint8_t get_idt_descriptor_dpl(IDT_GATE_DESCRIPTOR idt_gate);
void set_idt_descriptor_dpl(uint8_t dpl, IDT_GATE_DESCRIPTOR* pidt_gate);
uint8_t get_idt_descriptor_present(IDT_GATE_DESCRIPTOR idt_gate);
void set_idt_descriptor_present(IDT_GATE_DESCRIPTOR* pidt_gate);
uint8_t get_idt_descriptor_type(IDT_GATE_DESCRIPTOR idt_gate);
uint8_t get_idt_descriptor_selector_rpl(IDT_GATE_DESCRIPTOR idt_gate);
void  set_idt_descriptor_selector_rpl(IDT_GATE_DESCRIPTOR* pidt_gate, uint8_t rpl);
void backup_and_new_idt_gate_descriptors(unsigned char core, IDT_GATE_DESCRIPTOR *new_idt_table_gates, IDTR *backupidtrs);
void restore_idtrs(unsigned char core, IDTR *backupidtrs);
void save_rtc(void);
void restore_rtc(void);
void save_hpet(struct tagMyPageHelper page_helper);
void restore_hpet(struct tagMyPageHelper page_helper);
uint64_t save_tsc(void);
void restore_tsc(uint64_t tsc);
void user_isr_c_fun(void);
void hook_one_core_isr(unsigned char core, hook_isr_fun_type hookisr, unsigned char vector,\
                       IDT_GATE_DESCRIPTOR *new_idt_table_gates, IDTR *backupidtrs, uint64_t *origin_isr_addr);
void register_hook(void);
void unregister_hook(void);
#endif
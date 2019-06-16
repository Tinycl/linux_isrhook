#ifndef __ISRHELPER_H__
#define __ISRHELPER_H__

#include <linux/kernel.h> 
#include <linux/types.h>
#include <linux/module.h>
#include <asm/hpet.h>
#include "typehelper.h"
#include "pagehelper.h"

#define CORENUM 8

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

uint64_t idt_descriptor_offset_to_va(IDT_GATE_DESCRIPTOR idt_gate);
void va_to_idt_descriptor_offset(uint64_t va, IDT_GATE_DESCRIPTOR* pidat_gate);
uint8_t get_idt_descriptor_dpl(IDT_GATE_DESCRIPTOR idt_gate);
void set_idt_descriptor_dpl(uint8_t dpl, IDT_GATE_DESCRIPTOR* pidt_gate);
uint8_t get_idt_descriptor_present(IDT_GATE_DESCRIPTOR idt_gate);
void set_idt_descriptor_present(IDT_GATE_DESCRIPTOR* pidt_gate);
uint8_t get_idt_descriptor_type(IDT_GATE_DESCRIPTOR idt_gate);
uint8_t get_idt_descriptor_selector_rpl(IDT_GATE_DESCRIPTOR idt_gate);
void  set_idt_descriptor_selector_rpl(IDT_GATE_DESCRIPTOR* pidt_gate, uint8_t rpl);
void backup_idt_gate_descriptors(unsigned char core);


#endif